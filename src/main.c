#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <inttypes.h>

#include "defs.h"
#include "search.h"

S_Masks Masks;

void time_perft(S_Position* Pos, const char* FEN, int depth);
void time_search(S_Position* Pos, const char* FEN, int depth);

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
    memset(Pos.PV.nodes, 0, sizeof(u16) * MAX_SEARCH_DEPTH * MAX_SEARCH_DEPTH);
    memset(Pos.PV.length, 0, sizeof(u8) * MAX_SEARCH_DEPTH);


    u8 err = load_fen(&Pos,kiwipete);
    if (err)
        printf("Error while loading FEN string: %s\n", decodeFenError(err));

    // clock_t start = clock();
    // iterative_deepening(&Pos, 7);
    // clock_t end = clock();
    // printf("Search took: %f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);

    time_perft(&Pos, kiwipete, 5);
    // time_search(&Pos, kiwipete, 7);
    // uci_loop();

    free(TTable.entries);
}

void init_all(){
    init_masks();
    init_bb();
    init_TT();
    clear_killer_moves();
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
    printf("Search took: %f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);

    printf("info score cp %d depth %d nodes %"PRIu64" pv ", best_move.score, depth, total_nodes_searched);
    for (int i = 0; i < Pos->PV.length[0]; i++){
        printf("%s%s ", 
            squares_int_to_chr[MOVE_FROM_SQUARE(Pos->PV.nodes[0][i])],
            squares_int_to_chr[MOVE_TO_SQUARE(Pos->PV.nodes[0][i])]
        );  
    }
    printf("\n");
}