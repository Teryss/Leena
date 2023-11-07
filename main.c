#include <stdio.h>
// #include <sys/types.h>
#include <time.h>
#include "defs.h"

S_Masks Masks;

#define POSITION_TO_LOAD STARTING_POSITION_FEN
#define DEPTH 7
int main(){
    S_Board Board;
    load_fen(&Board, POSITION_TO_LOAD);
    init_masks();

    printf("Size of Board: %f kB\n", (double)sizeof(Board) / 1024);

    print_board(&Board);
    clock_t start = clock();
    u64 nodes = run_perft(&Board, DEPTH);
    clock_t end = clock();
    printf("Nodes searched: %lu\n", nodes);
    printf("Search took: %f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);
    printf("%0.2f MNodes/second\n", ((double)nodes / 1000000) / ((double)(end - start) / CLOCKS_PER_SEC));

    // print_bitboard(Board.occupied_squares_by[BOTH], NO_SQR);
    // update_occupied_squares(&Board);
    // print_bitboard(Board.occupied_squares_by[BOTH], NO_SQR);    
    // print_board(&Board);
}