#include "search.h"
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

u64 total_nodes_searched = 0;

S_Move search(S_Position* Pos, uint depth);
static i32 alpha_beta(S_Position* Pos, i32 alpha, i32 beta, i32 depth);
static i32 quiesence_search(S_Position* Pos, i32 alpha, i32 beta, i32 depth);

S_Move search(S_Position* Pos, uint depth){
    i32 best_eval = -INFINITE, eval, best_move_index = 0, alpha = -INFINITE, beta = INFINITE;
    Pos->ply = 0;

    S_Moves Moves;
    S_Board Board_copy;
    memcpy(&Board_copy, Pos->Board, sizeof(S_Board));
    generateMoves(Pos, &Moves);
    filter_illegal(Pos, &Moves);
    sort_moves(Pos, &Moves);
    u16 state = encode_state(Pos);

    Pos->PV.length[Pos->ply] = Pos->ply;

    for (int i = 0; i < Moves.count; i++){
        make_move(Pos, Moves.moves[i]);
        ASSERT(isKingPresent(Pos));

        total_nodes_searched++;
        eval = -alpha_beta(Pos, alpha, beta, depth - 1);

        memcpy(Pos->Board, &Board_copy, sizeof(S_Board));

        restore_state(Pos, state);
        if (eval > best_eval){
            best_eval = eval;
            best_move_index = i;

            Pos->PV.nodes[Pos->ply][Pos->ply] = Moves.moves[i];

            for (int next_ply = Pos->ply + 1; next_ply < Pos->PV.length[Pos->ply + 1]; next_ply++)
                Pos->PV.nodes[Pos->ply][next_ply] = Pos->PV.nodes[Pos->ply + 1][next_ply];
            
            Pos->PV.length[Pos->ply] = Pos->PV.length[Pos->ply + 1];
        }
        
        if (eval > alpha){
            alpha = eval;
        }
    }

    S_Move Best_move = {Moves.moves[best_move_index], best_eval};
    return Best_move;
}

i32 iterative_deepening(S_Position* Pos, uint max_depth){
    i32 best_eval = -INFINITE, eval, alpha, beta = INFINITE;
    Pos->ply = 0;

    S_Moves Moves;
    S_Board Board_copy;
    memcpy(&Board_copy, Pos->Board, sizeof(S_Board));
    generateMoves(Pos, &Moves);
    filter_illegal(Pos, &Moves);
    sort_moves(Pos, &Moves);
    u16 state = encode_state(Pos);

    for (int depth = 1; depth < max_depth + 1; depth++){
        Pos->PV.length[Pos->ply] = Pos->ply;
        alpha = -INFINITE;
        best_eval = -INFINITE;
        total_nodes_searched = 0;

        for (int i = 0; i < Moves.count; i++){
            make_move(Pos, Moves.moves[i]);
            total_nodes_searched++;
            eval = -alpha_beta(Pos, alpha, beta, depth - 1);

            memcpy(Pos->Board, &Board_copy, sizeof(S_Board));

            restore_state(Pos, state);
            if (eval > best_eval){
                best_eval = eval;

                Pos->PV.nodes[Pos->ply][Pos->ply] = Moves.moves[i];
                for (int next_ply = Pos->ply + 1; next_ply < Pos->PV.length[Pos->ply + 1]; next_ply++)
                    Pos->PV.nodes[Pos->ply][next_ply] = Pos->PV.nodes[Pos->ply + 1][next_ply];

                Pos->PV.length[Pos->ply] = Pos->PV.length[Pos->ply + 1];
            }
            
            if (eval > alpha){
                alpha = eval;
            }

        }
        sort_afterSearch(Pos, &Moves);
        printf("info score cp %d depth %d nodes %"PRIu64" pv ", best_eval, depth, total_nodes_searched);
        for (int i = 0; i < depth; i++){
            printf("%s%s ", 
                squares_int_to_chr[MOVE_FROM_SQUARE(Pos->PV.nodes[0][i])],
                squares_int_to_chr[MOVE_TO_SQUARE(Pos->PV.nodes[0][i])]
            );  
        }
        printf("\n");

    }

    return best_eval;
}

static i32 alpha_beta(S_Position* Pos, i32 alpha, i32 beta, i32 depth){
    Pos->PV.length[Pos->ply] = Pos->ply;
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
        // sort_afterSearch(Pos, &Moves);

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
        restore_state(Pos, state);
        
        if (eval > alpha){
            alpha = eval;
            node_type = NODE_EXACT;
            Pos->PV.nodes[Pos->ply][Pos->ply] = Moves.moves[i];

            for (int next_ply = Pos->ply + 1; next_ply < Pos->PV.length[Pos->ply + 1]; next_ply++)
                Pos->PV.nodes[Pos->ply][next_ply] = Pos->PV.nodes[Pos->ply + 1][next_ply];
            
            Pos->PV.length[Pos->ply] = Pos->PV.length[Pos->ply + 1];
        }

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