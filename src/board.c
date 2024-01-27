#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include "defs.h"

const char* pieces_int_to_chr = "pnbrqkPNBRQK-";
// const int piece_chr_to_int[] = {
//     ['p'] = 0, ['n'] = 1, ['b'] = 2, ['r'] = 3, ['q'] = 4, ['k'] = 5,
//     ['P'] = 6, ['N'] = 7, ['B'] = 8, ['R'] = 9, ['Q'] = 10, ['K'] = 11
// };

const char squares_int_to_chr[65][3] = {
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1", "NSQ"
};

// enum castling {wk = 1, wq = 2, bk = 4, bq = 8}, sum: 15;
const uint CASTLING_CHANGE_ON_MOVE[64] = {
    wq + wk + bk, 15, 15, 15, wq + wk, 15, 15, bq + wk + wq,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    bk + bq + wk, 15, 15, 15, bq + bk, 15, 15, bk + bq + wq,
};
static inline uint color(char c){
    if (c >= 'A' && c <= 'Z')
        return WHITE;
    return BLACK;
}

uint load_fen(S_Position* Pos, const char* const FEN){
    reset(Pos);
    enum fen_part {fenPieces, fenSideToMove, fenCastlePermission, fenEnPassant, fenFiftyMoves, fenPly};    
    uint fenPart = 0, sqr = 0; 

    const char* start = FEN, *end = FEN + strlen(FEN); 

    for (; start < end; start++){
        if (*start == ' '){
            fenPart++; 
            continue;
        }

        switch (fenPart) {
            case fenPieces:
                if (*start == '/'){
                    continue;
                }
                if (*start >= '0' && *start <= '8'){
                    sqr += *start - '0'; 
                    continue;
                }
                Pos->Board->pieceSet[sqr] = *start;
                SET_BIT(Pos->Board->piecesBB[pieceChrToInt(*start)], sqr);
                SET_BIT(Pos->Board->colorBB[color(*start)], sqr);
                sqr++;
                break;
            case fenSideToMove:
                switch (*start) {
                    case 'w':   Pos->sideToMove = WHITE; break;
                    case 'b':   Pos->sideToMove = BLACK; break;
                    default:    return WRONG_SIDE_TO_MOVE;
                }
                break;
            case fenCastlePermission:
                switch (*start) {
                    case '-':   continue; 
                    case 'k':   Pos->castlePermission |= bk; break;
                    case 'K':   Pos->castlePermission |= wk; break;
                    case 'q':   Pos->castlePermission |= bq; break;
                    case 'Q':   Pos->castlePermission |= wq; break;
                    default:    return WRONG_CASTLE_PERM;
                }
                break;
            case fenEnPassant:
                if (*start == '-')
                    Pos->enPassantSquare = 0;
                else if (*start + 1 != ' ' && *start + 2 == ' '){
                    Pos->enPassantSquare = ROW_COL_TO_SQR(
                        (8 - (int)(*(start + 1) - '0')), 
                        (*start - 'a')
                    );
                }else{
                }
                break;
            case fenFiftyMoves:
                if (*(start + 1) == ' ')
                    Pos->fiftyMovesCounter = *start - '0';
                else{
                    Pos->fiftyMovesCounter = 10 * (*start - '0') + *(start + 1) - '0';
                    start++;
                }
                break;
            case fenPly:
                if ((start + 1) >= end)
                    Pos->ply = *start - '0';
                else{
                    Pos->ply = 10 * (*start - '0') + *(start + 1) - '0';
                    start++;
                }
                break;
            default:
                return TOO_MANY_SPACES;
        }
    }
    Pos->Board->colorBB[BOTH] = Pos->Board->colorBB[WHITE] | Pos->Board->colorBB[BLACK]; 
    
    if (fenPart != 5)
        return NOT_ENOUGH_INFO;

    return SUCCESS;
}

void reset(S_Position* pos){
    pos->castlePermission = 0;
    pos->enPassantSquare = 0;
    pos->sideToMove = 0;
    pos->ply = 0;
    pos->fiftyMovesCounter = 0;
    memset(pos->Board->piecesBB, 0, sizeof(u64) * 9);
    memset(pos->Board->pieceSet, 0, sizeof(char) * 64);
    pos->Board->hash = 0;
}


/*
    This function is only used while parsing FEN string. It's too slow for usage during a search.
    After manually updating occupied squares in making/unmaking move functions, there is ~22% speedup!
*/
// void update_occupied_squares(S_Board* board) {
//     uint piece;
//     memset(board->colorBB, 0, sizeof(u64) * 3);

//     for (piece = 0; piece < 6; piece++){ board->occupied_squares_by[BLACK] |= board->pieces[piece]; }
//     for (piece = 6; piece < 12; piece++){ board->occupied_squares_by[WHITE] |= board->pieces[piece]; }
    
//     board->occupied_squares_by[BOTH] = board->occupied_squares_by[BLACK] | board->occupied_squares_by[WHITE];
// }

void print_board(S_Position* Pos){
    for (int r = 0; r < 8; r++){
        printf("%d ", 8 - r);

        for (int c = 0; c < 8; c++){
            printf(" %c ", 
                Pos->Board->pieceSet[ROW_COL_TO_SQR(r, c)] ? Pos->Board->pieceSet[ROW_COL_TO_SQR(r, c)] : '.');
        }
        printf("\n");
    }

    printf("\n   A  B  C  D  E  F  G  H\n\n");
    printf("Hash: %"PRIx64"\n",Pos->Board->hash);
    printf("En passant square: %s\n", Pos->enPassantSquare == 0 ? "-" : squares_int_to_chr[Pos->enPassantSquare]);
    printf("Castling rights: %c%c%c%c\n", 
        Pos->castlePermission & wk ? 'K' : '-',
        Pos->castlePermission & wq ? 'Q' : '-',
        Pos->castlePermission & bk ? 'k' : '-',
        Pos->castlePermission & bq ? 'q' : '-');
    printf("Side to move: %s\n", Pos->sideToMove == WHITE ? "White" : "Black");
    printf("Fifty moves counter: %d\n", Pos->fiftyMovesCounter);
    printf("Half moves total: %d\n", Pos->ply);
}

void print_bitboard(u64 bitboard){
    for (int r = 0; r < 8; r++){
        printf("%d  ", 8 - r);
        for (int c = 0; c < 8; c++){
            printf(" %c ", (GET_BIT(bitboard, ROW_COL_TO_SQR(r, c)) > 0 ? 'X' : '.'));
        }
        printf("\n");
    }
    printf("\n    A  B  C  D  E  F  G  H\n\n");
    printf("Value: %"PRIu64"\n\n", bitboard);
}