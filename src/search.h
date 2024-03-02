#ifndef SEARCH_H
#define SEARCH_H

#include "defs.h"

#define INFINITE 1000000
#define MAX_DEPTH_QUEIESENCE 4
#define NO_HASH_ENTRY 999991


S_Move search(S_Position* Pos, uint depth);
i32 iterative_deepening(S_Position* Pos, uint max_depth);

extern u64 total_nodes_searched;

static INLINE u64 get_TT_index(S_Board* Board){
    return Board->hash % TTable.count;
}

static INLINE void put_TT_entry(u64 position_hash, i32 eval, Node node_type, u8 depth){
    S_TT_Entry* entry = &TTable.entries[position_hash % TTable.count];

    // if (entry->depth < depth){
        entry->hash = position_hash;
        entry->score = eval;
        entry->node_type = node_type;
        entry->depth = depth;
    // }
}

static inline i32 get_TT_entry_score(S_Board* Board, i32 alpha, i32 beta, i32 depth){
    S_TT_Entry* entry = &TTable.entries[get_TT_index(Board)];
    
    if (entry->hash == Board->hash){
        if (entry->depth >= depth){
            switch (entry->node_type) {
                case NODE_NONE:
                    return NO_HASH_ENTRY;
                case NODE_EXACT:
                    return entry->score;
                case NODE_UPPER:
                    if (entry->score <= alpha){
                        return alpha;
                    }
                    break;
                case NODE_LOWER:
                    if (entry->score >= beta){
                        return beta;
                    }
                    break;
            }
        }
    }
    return NO_HASH_ENTRY;
}

static INLINE u8 isReadyToPromote(S_Position* Pos){
    static const u64 first_rank = 0xFFLLU; 
    static const u8 seventh_rank_multiplier = 8 * 6;
    static const u8 seventh_to_second_rank = 8 * 5;
    if ((first_rank << (seventh_rank_multiplier - seventh_to_second_rank * Pos->sideToMove)) & Pos->Board->piecesBB[p] & Pos->Board->colorBB[Pos->sideToMove]){
        return 1;
    }
    return 0;
}

static INLINE void sort_afterSearch(const S_Position* const Pos, S_Moves* Moves){
    u16 temp_move;

    for (int i = 0; i < Moves->count; i++){
        if (Moves->moves[i] == Pos->PV.nodes[0][Pos->ply]){
            temp_move = Moves->moves[0];
            Moves->moves[0] = Moves->moves[i];
            Moves->moves[i] = temp_move;
            return;
        }
    }
            // printf("Ply: %i\n", Pos->ply);
            // print_move(Pos->Board, Moves->moves[0]);
}

#endif