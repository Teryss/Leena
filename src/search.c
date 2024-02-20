#include "defs.h"
// #include <stdio.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define INFINITE 1000000
#define MAX_DEPTH_QUEIESENCE 6
#define NO_HASH_ENTRY 999991

u64 total_nodes_searched = 0;

S_Move search(S_Position* Pos, uint depth);
static i32 alpha_beta(S_Position* Pos, i32 alpha, i32 beta, i32 depth);
static i32 quiesence_search(S_Position* Pos, i32 alpha, i32 beta, i32 depth);

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
    // else
    //     hash_collision++;
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

u64 isKingPresent(S_Position* Pos){
    return COUNT_BITS(Pos->Board->piecesBB[k] & Pos->Board->colorBB[1 - Pos->sideToMove]);
}

S_Move search(S_Position* Pos, uint depth){
    i32 best_eval = -INFINITE, eval, best_move_index = 0, alpha = -INFINITE, beta = INFINITE;

    S_Moves Moves;
    S_Board Board_copy;
    memcpy(&Board_copy, Pos->Board, sizeof(S_Board));
    generateMoves(Pos, &Moves);
    filter_illegal(Pos, &Moves);
    sort_moves(Pos, &Moves);
    u16 state = encode_state(Pos);

    for (int i = 0; i < Moves.count; i++){
        make_move(Pos, Moves.moves[i]);
        ASSERT(isKingPresent(Pos));

        total_nodes_searched++;
        eval = -alpha_beta(Pos, alpha, beta, depth - 1);

        if (eval > best_eval){
            best_eval = eval;
            best_move_index = i;
        }
        
        if (eval > alpha)
            alpha = eval;

        memcpy(Pos->Board, &Board_copy, sizeof(S_Board));
        restore_state(Pos, state);
    }

    S_Move Best_move = {Moves.moves[best_move_index], best_eval};
    return Best_move;
}

static i32 alpha_beta(S_Position* Pos, i32 alpha, i32 beta, i32 depth){
    i32 eval;
    if ((eval = get_TT_entry_score(Pos->Board, alpha, beta, depth)) != NO_HASH_ENTRY){
        return eval;
    }

    if (depth == 0) {
        // return evaluate(Pos);
        return quiesence_search(Pos, alpha, beta, MAX_DEPTH_QUEIESENCE); 
    }
    int node_type = NODE_LOWER;
    S_Moves Moves;
    S_Board Board_copy;
    memcpy(&Board_copy, Pos->Board, sizeof(S_Board));
    
    generateMoves(Pos, &Moves);
    filter_illegal(Pos, &Moves);
    sort_moves(Pos, &Moves);

    u16 state = encode_state(Pos);

    for (int i = 0; i < Moves.count; i++){
        make_move(Pos, Moves.moves[i]);
        total_nodes_searched++;
        eval = -alpha_beta(Pos, -beta, -alpha, depth - 1);

        if (eval >= beta){
            if (MOVE_GET_FLAG(Moves.moves[i]) < CAPTURE){
                killer_moves[Pos->ply][1] = killer_moves[Pos->ply][0];
                killer_moves[Pos->ply][0] = Moves.moves[i];
            }
            put_TT_entry(Pos->Board->hash, beta, NODE_UPPER, depth);
            restore_state(Pos, state);
            return beta;
        }
        
        if (eval > alpha){
            alpha = eval;
            node_type = NODE_EXACT;
        }

        restore_state(Pos, state);
        memcpy(Pos->Board, &Board_copy, sizeof(S_Board));
    }
    put_TT_entry(Pos->Board->hash, alpha, node_type, depth);
    return alpha;
}

static i32 quiesence_search(S_Position* Pos, i32 alpha, i32 beta, i32 depth){
    if (depth == 0)
        return evaluate(Pos);

    i32 eval = evaluate(Pos);
    if (eval >= beta)
        return beta;
    if (alpha < eval)
        alpha = eval;
    
    int delta = 900;
    if (isReadyToPromote(Pos))
        delta += 700;
    if (eval < alpha - delta)
        return alpha;

    S_Moves Moves;
    generateOnlyCaptures(Pos, &Moves);
    filter_illegal(Pos, &Moves);
    if (Moves.count == 0)
        return eval;

    S_Board Board_copy;
    u16 state = encode_state(Pos);
    sort_captures(Pos, &Moves);
    memcpy(&Board_copy, Pos->Board, sizeof(S_Board));

    for (int i = 0; i < Moves.count; i++){
        make_move(Pos, Moves.moves[i]);
        eval = -quiesence_search(Pos, -beta, -alpha, depth - 1);
        restore_state(Pos, state);
        total_nodes_searched++;
        if (eval >= beta)
            return beta;
        if (eval > alpha)
            alpha = eval;
        memcpy(Pos->Board, &Board_copy, sizeof(S_Board));
    }

    return alpha;
}