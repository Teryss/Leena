// #include "defs.h"
// #include <stdio.h>
// #include <string.h>

// #define INFINITE 1000000
// #define MAX_DEPTH_QUEIESENCE 6
// #define NO_HASH_ENTRY 123456

// u64 hash_collision = 0;
// u64 total_nodes_searched = 0;

// S_Move search(S_Board* Board, uint depth);
// i32 alpha_beta(S_Board* Board, i32 alpha, i32 beta, i32 depth);
// i32 quiesence_search(S_Board* Board, i32 alpha, i32 beta, i32 depth);

// static INLINE u64 get_TT_index(S_Board* Board){
//     return Board->hash % TTable.count;
// }

// static INLINE void put_TT_entry(u64 position_hash, i32 eval, Node node_type, u8 depth){
//     S_TT_Entry* entry = &TTable.entries[position_hash % TTable.count];

//     // if (entry->depth < depth){
//         entry->hash = position_hash;
//         entry->score = eval;
//         entry->node_type = node_type;
//         entry->depth = depth;
//     // }
// }

// static inline i32 get_TT_entry_score(S_Board* Board, i32 alpha, i32 beta, i32 depth){
//     S_TT_Entry* entry = &TTable.entries[get_TT_index(Board)];
    
//     if (entry->hash == Board->hash){
//         if (entry->depth >= depth){
//             switch (entry->node_type) {
//                 case NODE_NONE:
//                     return NO_HASH_ENTRY;
//                 case NODE_EXACT:
//                     return entry->score;
//                 case NODE_UPPER:
//                     if (entry->score <= alpha){
//                         return alpha;
//                     }
//                     break;
//                 case NODE_LOWER:
//                     if (entry->score >= beta){
//                         return beta;
//                     }
//                     break;
//             }
//         }
//     }
//     else
//         hash_collision++;
//     return NO_HASH_ENTRY;
// }

// static INLINE u8 isReadyToPromote(S_Board* Board){
//     const u64 first_rank = 0xFFLLU; 
//     const u8 seventh_rank_multiplier = 8 * 6;
//     const u8 seventh_to_second_rank = 8 * 5;
//     if ((first_rank << (seventh_rank_multiplier - seventh_to_second_rank * Board->sideToMove)) & Board->pieces[p + 6 * Board->sideToMove]){
//         return 1;
//     }
//     return 0;
// }

// S_Move search(S_Board* Board, uint depth){
//     S_Moves Moves;
//     S_Board Board_copy;
//     i32 best_eval = -INFINITE, eval, best_move_index = 0, alpha = -INFINITE, beta = INFINITE;

//     generateMoves(Board, &Moves);
//     sort_moves(Board, &Moves);
//     memcpy(&Board_copy, Board, sizeof(S_Board));

//     for (int i = 0; i < Moves.count; i++){
//         make_move(Board, Moves.moves[i]);
//         if ((is_king_attacked(Board)) == 0){
//             total_nodes_searched++;
//             eval = -alpha_beta(Board, alpha, beta, depth - 1);
//             if (eval > best_eval){
//                 best_eval = eval;
//                 best_move_index = i;
//             }
            
//             if (eval > alpha)
//                 alpha = eval;

//         }
//         memcpy(Board, &Board_copy, sizeof(S_Board));
//     }

//     S_Move Best_move = {Moves.moves[best_move_index], best_eval};
//     return Best_move;
// }

// i32 alpha_beta(S_Board* Board, i32 alpha, i32 beta, i32 depth){
//     i32 eval;
//     if ((eval = get_TT_entry_score(Board, alpha, beta, depth)) != NO_HASH_ENTRY){
//         return eval;
//     }

//     if (depth == 0) { 
//         return quiesence_search(Board, alpha, beta, MAX_DEPTH_QUEIESENCE);
//     }

//     S_Moves Moves;
//     S_Board Board_copy;
//     int node_type = NODE_LOWER;

//     generateMoves(Board, &Moves);
//     sort_moves(Board, &Moves);
//     memcpy(&Board_copy, Board, sizeof(S_Board));

//     for (int i = 0; i < Moves.count; i++){
//         make_move(Board, Moves.moves[i]);
//         if ((is_king_attacked(Board)) == 0){
//             total_nodes_searched++;
//             eval = -alpha_beta(Board, -beta, -alpha, depth - 1);
//             if (eval >= beta){
//                 if (MOVE_GET_FLAG(Moves.moves[i]) < CAPTURE){
//                     killer_moves[Board->ply--][1] = killer_moves[Board->ply--][0];
//                     killer_moves[Board->ply--][0] = Moves.moves[i];
//                 }
//                 put_TT_entry(Board->hash, beta, NODE_UPPER, depth);
//                 return beta;
//             }
//             if (eval > alpha){
//                 alpha = eval;
//                 node_type = NODE_EXACT;
//             }

//         }
//         memcpy(Board, &Board_copy, sizeof(S_Board));
//     }
//     put_TT_entry(Board->hash, alpha, node_type, depth);
//     return alpha;
// }

// i32 quiesence_search(S_Board* Board, i32 alpha, i32 beta, i32 depth){
//     if (depth == 0)
//         return evaluate(Board);

//     i32 eval = evaluate(Board);
//     if (eval >= beta)
//         return beta;
//     if (alpha < eval)
//         alpha = eval;
    
//     int delta = 900;
//     if (isReadyToPromote(Board))
//         delta += 700;
//     if (eval < alpha - delta)
//         return alpha;

//     S_Moves Moves;
//     generateOnlyCaptures(Board, &Moves);
//     if (Moves.count == 0)
//         return eval;

//     S_Board Board_copy;
//     sort_captures(Board, &Moves);
//     memcpy(&Board_copy, Board, sizeof(S_Board));

//     for (int i = 0; i < Moves.count; i++){
//         make_move(Board, Moves.moves[i]);
//         if ((is_king_attacked(Board)) == 0){
//             total_nodes_searched++;
//             eval = -quiesence_search(Board, -beta, -alpha, depth - 1);
//             if (eval >= beta)
//                 return beta;
//             if (eval > alpha)
//                 alpha = eval;
//         }
//         memcpy(Board, &Board_copy, sizeof(S_Board));
//     }

//     return alpha;
// }