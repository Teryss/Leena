#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "defs.h"
#include <inttypes.h>

S_Masks Masks;

// void init_all();
// void time_perft(S_Position* Pos, const char* FEN, int depth);
// void time_search(S_Position* Pos, const char* FEN, int depth);
void comparePositions(S_Position* Pos, S_Position* PreviousPosition);

static inline const char* decodeFenError(u8 flag){
    switch (flag){
        case TOO_MANY_SPACES:
            return "TOO_MANY_SPACES";
        case NOT_ENOUGH_INFO:
            return "NOT_ENOUGH_INFO";
        case WRONG_SIDE_TO_MOVE:
            return "WRONG_SIDE_TO_MOVE";
        case WRONG_CASTLE_PERM:
            return "WRONG_CASTLE_PERM";
        case WRONG_EN_PASSANT:
            return "WRONG_EN_PASSANT";
        default:
            return "UNKNOWN_ERROR";
    }
} 

int main(){
    S_Board Board;
    S_Position Pos = {.Board = &Board};
    init_all();

    uint err = load_fen(&Pos, "r3k2r/pPppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
    if (err)
        printf("Error while loading FEN string: %s\n", decodeFenError(err));

    S_Position* PosCopy = malloc(sizeof(S_Position));
    memcpy(PosCopy, &Pos, sizeof(S_Position));
    S_Board* BoardCopy = malloc(sizeof(S_Board));
    memcpy(BoardCopy, Pos.Board, sizeof(S_Board));
    PosCopy->Board = BoardCopy;

    S_Moves Moves;
    generateMoves(&Pos, &Moves);
    u16 state = encode_state(&Pos);
    u8 capturePiece;
    for (int i = 0; i < 1; i++){
        printf("Index: %i ", i);
        print_move(Pos.Board, Moves.moves[i]);
        capturePiece = make_move(&Pos, Moves.moves[i]);
        print_board(&Pos);
        undo_move(&Pos, Moves.moves[i], state, capturePiece);
        comparePositions(&Pos, PosCopy);
    }

    print_board(&Pos);
    // for (int i = 0; i < 64; i++){
    //     if (Pos.Board->pieceSet[i] != PosCopy->Board->pieceSet[i]){
    //         printf("Square: %s, Is [%c:%i], should be [%c:%i]\n", squares_int_to_chr[i], Pos.Board->pieceSet[i], Pos.Board->pieceSet[i], PosCopy->Board->pieceSet[i], PosCopy->Board->pieceSet[i]);
    //     }
    // }

    for (int i = 0; i < 6; i++){
        if (Pos.Board->piecesBB[i] != PosCopy->Board->piecesBB[i]){
            printf("Piece: %c\n", pieces_int_to_chr[i]);
            print_bitboard(Pos.Board->piecesBB[i]);
            print_bitboard(PosCopy->Board->piecesBB[i]);
        }
    }

    // u8 capturePiece = make_move(&Pos, Moves.moves[1]);
    // undo_move(&Pos, Moves.moves[1], state, capturePiece);
    // comparePositions(&Pos, PosCopy);

    // print_moves(&Pos, &Moves);

    // make_move(&Pos, Moves.moves[0]);
    // printf("####################################################\n");
    // print_bitboard(Pos.Board->colorBB[WHITE], NO_SQR);
    // print_bitboard(Pos.Board->colorBB[BLACK], NO_SQR);
    // print_bitboard(Pos.Board->colorBB[BOTH], NO_SQR);
    // print_board(&Pos);
    // print_bitboard(u64 bitboard, int current_pos)
    // init_all();
    // load_fen(&Board, "k2r4/8/7q/b7/5B2/8/3K4/8 w - - 0 1");
    // load_fen(&Board, "k7/8/8/8/4B3/8/2rK4/8 w - - 0 1");

    // perft_suite(&Board);

    // free(PosCopy);
    // free(BoardCopy);
    // free(TTable.entries);
}

void init_all(){
    init_masks();
    init_bb();
//     init_TT();
//     clear_killer_moves();
}

void comparePositions(S_Position* Pos, S_Position* PreviousPosition){
    printf("== 0 means they're equal ==\nPosition comparison:\nWhole position: %d\nBoard*: %d\nCastlePermission: %d\nenPassantSquare: %d\nfiftyMoves: %d\nply: %d\nsideToMove: %d\nstateHistory: %d\n\n",
        memcmp(Pos, PreviousPosition, sizeof(S_Position)),
        Pos->Board != PreviousPosition->Board,
        Pos->castlePermission != PreviousPosition->castlePermission,
        Pos->enPassantSquare != PreviousPosition->enPassantSquare,
        Pos->fiftyMovesCounter != PreviousPosition->fiftyMovesCounter,
        Pos->ply != PreviousPosition->ply,
        Pos->sideToMove != PreviousPosition->sideToMove,
        memcmp(Pos->stateHistory, PreviousPosition->stateHistory, sizeof(u16) * MAX_GAME_SIZE)
    );

    printf("Board comparison:\nWhole board: %d\n", memcmp(Pos->Board, PreviousPosition->Board, sizeof(S_Board)));
    printf("PiecesBB: %d\nColorBB: %d\nPieceSet: %d\nHash: %d\n", 
        memcmp(Pos->Board->piecesBB, PreviousPosition->Board->piecesBB, sizeof(u64) * 6),
        memcmp(Pos->Board->colorBB, PreviousPosition->Board->colorBB, sizeof(u64) * 3),
        memcmp(Pos->Board->pieceSet, PreviousPosition->Board->pieceSet, sizeof(u8) * 64),
        Pos->Board->hash != PreviousPosition->Board->hash
    );
    printf("====\n");
}

// void time_perft(S_Position* Pos, const char* FEN, int depth){
//     load_fen(Pos, FEN);
//     clock_t start = clock();
//     u64 nodes = perft(Pos->Board, depth);
//     clock_t end = clock();
//     printf("Nodes searched: %"PRIu64"\n", nodes);
//     printf("Search took: %f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);
//     printf("%0.2f MNodes/second\n", ((double)nodes / 1000000) / ((double)(end - start) / CLOCKS_PER_SEC));
// }

// void time_search(S_Position* Pos, const char* FEN, int depth){
//     load_fen(Pos, FEN);
//     clock_t start = clock();
//     S_Move best_move = search(Pos->Board, depth);
//     clock_t end = clock();
//     print_move(best_move.move);
//     printf("Score: %0.2f\n", (double)best_move.score/100);
//     printf("Search took: %f seconds, total nodes: %"PRIu64"\n", (double)(end - start) / CLOCKS_PER_SEC, total_nodes_searched);
//     printf("%0.2f MNodes/second\n", ((double)total_nodes_searched / 1000000) / ((double)(end - start) / CLOCKS_PER_SEC));
// }