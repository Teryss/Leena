#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <inttypes.h>

#include "defs.h"

S_Masks Masks;

// void init_all();
void time_perft(S_Position* Pos, const char* FEN, int depth);
void time_search(S_Position* Pos, const char* FEN, int depth);
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

    u8 err = load_fen(&Pos,kiwipete);
    if (err)
        printf("Error while loading FEN string: %s\n", decodeFenError(err));


    // perft_suite(&Pos);
    // print_board(&Pos);
    time_search(&Pos, kiwipete, 7);
    // time_perft(&Pos, kiwipete, 6);

    // free(TTable.entries);
}

void init_all(){
    init_masks();
    init_bb();
//     init_TT();
    clear_killer_moves();
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

void time_perft(S_Position* Pos, const char* FEN, int depth){
    load_fen(Pos, FEN);
    clock_t start = clock();
    u64 nodes = perft(Pos, depth);
    clock_t end = clock();
    printf("Nodes searched: %"PRIu64"\n", nodes);
    printf("Search took: %f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);
    printf("%0.2f MNodes/second\n", ((double)nodes / 1000000) / ((double)(end - start) / CLOCKS_PER_SEC));
}

void time_search(S_Position* Pos, const char* FEN, int depth){
    load_fen(Pos, FEN);
    clock_t start = clock();
    S_Move best_move = search(Pos, depth);
    clock_t end = clock();
    print_move(Pos->Board, best_move.move);
    printf("Score: %0.2f\n", (double)best_move.score/100);
    printf("Search took: %f seconds, total nodes: %"PRIu64"\n", (double)(end - start) / CLOCKS_PER_SEC, total_nodes_searched);
    printf("%0.2f MNodes/second\n", ((double)total_nodes_searched / 1000000) / ((double)(end - start) / CLOCKS_PER_SEC));
}