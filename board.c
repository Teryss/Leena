#include <string.h>
#include <stdio.h>
#include "defs.h"

const char* pieces_int_to_chr = "pnbrqkPNBRQK-";
const int piece_chr_to_int[] = {
    ['p'] = 0, ['n'] = 1, ['b'] = 2, ['r'] = 3, ['q'] = 4, ['k'] = 5,
    ['P'] = 6, ['N'] = 7, ['B'] = 8, ['R'] = 9, ['Q'] = 10, ['K'] = 11
};

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

static uint get_next_space_FEN(const char * FEN, uint current_index, uint lenght){
    uint index = current_index;
    for (; index < lenght; index++){
        if (FEN[index] == ' '){
            return index;
        }
    }
    return index;
}

static uint pow_of_ten(uint multp){
    uint res = 1;
    while (multp) {
        res *= 10;
        multp--;
    }
    return res;
}

uint load_fen(S_Board* board, const char* const FEN){
    reset(board);
    uint square = 0, last_iteration_FEN_index = 0, FEN_length = strlen(FEN), next_space_index;
    char piece = ' ';

    #define check_if_index_is_outside_of_FEN(FEN_length, current_index, forward) if(current_index + forward > FEN_length) { return; }

    for (int i = 0; i < FEN_length; i++){
        piece = FEN[i];
        if (piece > 'a' && piece < 'z'){
            SET_BIT(board->pieces[piece_chr_to_int[piece]], square);
            SET_BIT(board->occupied_squares_by[BLACK], square);
            SET_BIT(board->occupied_squares_by[BOTH], square);
            square++;
        }else if (piece > 'A' && piece < 'Z'){
            SET_BIT(board->pieces[piece_chr_to_int[piece]], square);
            SET_BIT(board->occupied_squares_by[WHITE], square);
            SET_BIT(board->occupied_squares_by[BOTH], square);
            square++;
        }else if (piece > '0' && piece < '9'){
            square += piece - '0';
        }else if (piece == '/'){ continue; }
        else{ last_iteration_FEN_index = i + 1; break; }
    }

    next_space_index = get_next_space_FEN(FEN, last_iteration_FEN_index, FEN_length);
    if (next_space_index - last_iteration_FEN_index > 1){
        return load_fen_failure;
    }

    if (FEN[last_iteration_FEN_index] == 'w') { board->sideToMove = WHITE; }
    else {board->sideToMove = BLACK;}
    last_iteration_FEN_index += 2;
    
    for (int i = last_iteration_FEN_index; i < FEN_length; i++){
        switch (FEN[i]) {
            case 'K': board->castlePermission |= wk; break;
            case 'Q': board->castlePermission |= wq; break;
            case 'k': board->castlePermission |= bk; break;
            case 'q': board->castlePermission |= bq; break;
        }
        if (FEN[i] == ' '){ last_iteration_FEN_index += i - last_iteration_FEN_index; break; }
    }

    last_iteration_FEN_index++;
    next_space_index = get_next_space_FEN(FEN, last_iteration_FEN_index, FEN_length);
    if (next_space_index - last_iteration_FEN_index == 1){
        if (FEN[last_iteration_FEN_index] != '-'){
            return load_fen_failure;
        }
    }else if (next_space_index - last_iteration_FEN_index == 2){        
        if (FEN[last_iteration_FEN_index] >= 'a' && FEN[last_iteration_FEN_index] <= 'z'){
            if (FEN[last_iteration_FEN_index + 1] > '0' && FEN[last_iteration_FEN_index + 1] < '9'){
                board->enPassantSquare = ROW_COL_TO_SQR((8 - (int)(FEN[last_iteration_FEN_index + 1] - '0')), (FEN[last_iteration_FEN_index] - 'a'));
                last_iteration_FEN_index++;
            }
        }

        if (last_iteration_FEN_index + 1 != next_space_index) { return load_fen_failure; }
    }else {
        return load_fen_failure;
    }

    last_iteration_FEN_index +=2;
    next_space_index = get_next_space_FEN(FEN, last_iteration_FEN_index, FEN_length);
    
    if (next_space_index - last_iteration_FEN_index > 2){
        return load_fen_failure;
    }
    if (FEN[last_iteration_FEN_index] > '0' && FEN[last_iteration_FEN_index] < '9'){ 
        if (FEN[last_iteration_FEN_index + 1] == ' '){
            board->fiftyMovesCounter = FEN[last_iteration_FEN_index] - '0';
        }else{
            board->fiftyMovesCounter = (FEN[last_iteration_FEN_index] - '0') * 10 + FEN[last_iteration_FEN_index + 1] - '0';
            last_iteration_FEN_index++;
        }
    }

    last_iteration_FEN_index += 2;
    next_space_index = get_next_space_FEN(FEN, last_iteration_FEN_index, FEN_length);
    while (last_iteration_FEN_index != next_space_index){
        if (FEN[last_iteration_FEN_index] > '0' && FEN[last_iteration_FEN_index] < '9'){
            board->ply += (FEN[last_iteration_FEN_index] - '0') * pow_of_ten(next_space_index - 1 - last_iteration_FEN_index);
            last_iteration_FEN_index++;
        }else {
            break;
        }
    }

    return load_fen_success;
}

void reset(S_Board* board){
    board->castlePermission = 0;
    board->enPassantSquare = NO_SQR;
    board->sideToMove = 0;
    board->ply = 0;
    board->fiftyMovesCounter = 0;
    for (int i = 0; i < 12; i++){
        board->pieces[i] = 0;
    }
    for (int i = 0; i < 3; i++){
        board->occupied_squares_by[i] = 0;
    }
}


/*
    This function is only used while parsing FEN string. It's too slow for usage during a search.
    After manually updating occupied squares in making/unmaking move functions, there is ~22% speedup!
*/
void update_occupied_squares(S_Board* board) {
    uint piece;
    memset(board->occupied_squares_by, 0, sizeof(u64) * 3);

    for (piece = 0; piece < 6; piece++){ board->occupied_squares_by[BLACK] |= board->pieces[piece]; }
    for (piece = 6; piece < 12; piece++){ board->occupied_squares_by[WHITE] |= board->pieces[piece]; }
    
    board->occupied_squares_by[BOTH] = board->occupied_squares_by[BLACK] | board->occupied_squares_by[WHITE];
}

void print_board(S_Board* board){
    int found_piece;
    for (int r = 0; r < 8; r++){
        printf("%d  ", 8 - r);
        for (int c = 0; c < 8; c++){
            found_piece = 0;
            for (int piece = p; piece <= K; piece++){
                if (GET_BIT(board->pieces[piece], ROW_COL_TO_SQR(r, c))){
                    printf(" %c ", pieces_int_to_chr[piece]);
                    found_piece = 1; break;
                }
            }
            if (!found_piece) { printf(" . "); }
        }
        printf("\n");
    }
    printf("\n    A  B  C  D  E  F  G  H\n\n");
    printf("Value: %lu\n\n", board->occupied_squares_by[BOTH]);
    printf("En passant square: %s\n", board->enPassantSquare == NO_SQR ? "-" : squares_int_to_chr[board->enPassantSquare]);
    printf("Castling rights: %c%c%c%c\n", 
        board->castlePermission & wk ? 'K' : '-',
        board->castlePermission & wq ? 'Q' : '-',
        board->castlePermission & bk ? 'k' : '-',
        board->castlePermission & bq ? 'q' : '-');
    printf("Side to move: %s\n", board->sideToMove == WHITE ? "White" : "Black");
    printf("Fifty moves counter: %d\n", board->fiftyMovesCounter);
    printf("Half moves total: %d\n", board->ply);
}
void print_bitboard(u64 bitboard, int current_pos){
    for (int r = 0; r < 8; r++){
        printf("%d  ", 8 - r);
        for (int c = 0; c < 8; c++){
            if (ROW_COL_TO_SQR(r, c) == current_pos) { printf(" O "); }
            else{
                printf(" %c ", (GET_BIT(bitboard, ROW_COL_TO_SQR(r, c)) > 0 ? 'X' : '.'));
            }
        }
        printf("\n");
    }
    printf("\n    A  B  C  D  E  F  G  H\n\n");
    printf("Value: %lu\n\n", bitboard);
}