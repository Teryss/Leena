#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include "defs.h"
#include <inttypes.h>

S_Masks Masks;

// void init_all();
// void time_perft(S_Position* Pos, const char* FEN, int depth);
// void time_search(S_Position* Pos, const char* FEN, int depth);

int main(){
    S_Board Board;
    S_Position Pos = {.Board = &Board};
    uint err = load_fen(&Pos, STARTING_POSITION_FEN);
    
    if (err)
        printf("Error while loading FEN string: %u\n", err);
    print_board(&Pos);
    // print_bitboard(u64 bitboard, int current_pos)
    // init_all();
    // load_fen(&Board, "k2r4/8/7q/b7/5B2/8/3K4/8 w - - 0 1");
    // load_fen(&Board, "k7/8/8/8/4B3/8/2rK4/8 w - - 0 1");

    // perft_suite(&Board);


    // free(TTable.entries);
}

// void init_all(){
//     init_masks();
//     init_bb();
//     init_TT();
//     clear_killer_moves();
// }

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