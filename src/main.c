#include <stdio.h>
#include <time.h>
#include "defs.h"

S_Masks Masks;

#define POSITION_TO_LOAD "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define DEPTH 6

void init_all();
void time_perft(S_Board* Board, const char* FEN, int depth);
void time_search(S_Board* Board, const char* FEN, int depth);

int main(){
    S_Board Board;
    init_all();

    load_fen(&Board, kiwipete);
    // print_board(&Board);

    // printf("Evaluation: %i\n", evaluate(&Board));
    time_search(&Board, kiwipete, 8);
    // perft_suite(&Board);
    // time_perft(&Board, STARTING_POSITION_FEN, 7);
    // time_perft(&Board, kiwipete, 6);

    free(TTable.entries);
}

void init_all(){
    init_masks();
    init_TT();
    clear_killer_moves();
}

void time_perft(S_Board* Board, const char* FEN, int depth){
    load_fen(Board, FEN);
    clock_t start = clock();
    u64 nodes = perft(Board, depth);
    clock_t end = clock();
    printf("Nodes searched: %lu\n", nodes);
    printf("Search took: %f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);
    printf("%0.2f MNodes/second\n", ((double)nodes / 1000000) / ((double)(end - start) / CLOCKS_PER_SEC));
}

void time_search(S_Board* Board, const char* FEN, int depth){
    load_fen(Board, FEN);
    clock_t start = clock();
    S_Move best_move = search(Board, depth);
    clock_t end = clock();
    print_move(best_move.move);
    printf("Score: %0.2f\n", (double)best_move.score/100);
    printf("Search took: %f seconds, total nodes: %lu\n", (double)(end - start) / CLOCKS_PER_SEC, total_nodes_searched);
    printf("%0.2f MNodes/second\n", ((double)total_nodes_searched / 1000000) / ((double)(end - start) / CLOCKS_PER_SEC));
}