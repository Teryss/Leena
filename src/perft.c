#include "defs.h"
#include <stdio.h>
#include <inttypes.h>
#include <string.h>

u64 run_perft(S_Position* Pos, uint depth);

void perft_suite(S_Position* Pos){
    enum {NUMBER_OF_POSITIONS = 8};

    uint correct = 0;

    const char* positions[NUMBER_OF_POSITIONS] = {
        STARTING_POSITION_FEN, 
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -",
        "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - ",
        "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
        "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8  ",
        "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
        "n1n5/PPPk4/8/8/8/8/4Kppp/5N1N b - - 0 1"
    };
    const uint dephts[NUMBER_OF_POSITIONS] = {6, 5, 7, 6, 5, 5, 5, 6};
    const u64 expected_nodes[NUMBER_OF_POSITIONS] = {119060324, 193690690, 178633661, 706045033, 89941194, 164075551, 193690690, 71179139};
    u64 nodes;

    printf("Running perft test suite...\n");

    for (int i = 0; i < NUMBER_OF_POSITIONS; i++){
        printf("Position: %s\n", positions[i]);
        load_fen(Pos, positions[i]);
        nodes = run_perft(Pos, dephts[i]);
        if (nodes != expected_nodes[i]){
            printf("Failed, depth: %u, expected nodes: %"PRIu64", got: %"PRIu64"\n", dephts[i], expected_nodes[i], nodes);
        }else {
            correct++;
            printf("Success\n");
        }
    }

    if (correct == NUMBER_OF_POSITIONS){
        printf("Test passed! %u/%u were correct\n", correct, correct);
    }else{
        printf("Test failed! Only %u/%u were correct\n", correct, NUMBER_OF_POSITIONS);
    }
}

u64 perft(S_Position* Pos, uint depth){
    u64 nodes = 0ULL, current_nodes;

    S_Moves Moves;
    generateMoves(Pos, &Moves);
    // filter_illegal(Pos, &Moves);
    // memcpy(&Board_copy, Board, sizeof(S_Board));
    printf("Nodes searched per move - depth: %u\n", depth);

    u8 capturePiece;
    const u16 state = encode_state(Pos);
    for (int i = 0; i < Moves.count; i++){
        current_nodes = 0ULL;
        capturePiece = make_move(Pos, Moves.moves[i]);
        
        // if (!(is_king_attacked(Pos))){
            current_nodes += run_perft(Pos, depth - 1);
        // }

        if (current_nodes != 0){
            printf("%s%s",
                squares_int_to_chr[MOVE_FROM_SQUARE(Moves.moves[i])],
                squares_int_to_chr[MOVE_TO_SQUARE(Moves.moves[i])]
            );
            switch (MOVE_GET_FLAG(Moves.moves[i])) {
                case PROMOTION_B: printf("b"); break;
                case PROMOTION_R: printf("r"); break;
                case PROMOTION_N: printf("n"); break;
                case PROMOTION_Q: printf("q"); break;
                case CAPTURE_PROMOTION_B: printf("b"); break;
                case CAPTURE_PROMOTION_R: printf("r"); break;
                case CAPTURE_PROMOTION_N: printf("n"); break;
                case CAPTURE_PROMOTION_Q: printf("q"); break;
                default: break;
            }
            printf(": %"PRIu64"\n", current_nodes);
        }

        undo_move(Pos, Moves.moves[i], state, capturePiece);
        // memcpy(Board, &Board_copy, sizeof(S_Board));
        nodes += current_nodes;
    }


    return nodes;
}

u64 run_perft(S_Position* Pos, uint depth){
    if (depth == 0 || is_king_attacked(Pos)) { return 1ULL; }

    u64 nodes = 0ULL;
    S_Moves Moves;
    generateMoves(Pos, &Moves);
    // filter_illegal(Pos, &Moves);

    u8 capturePiece;
    const u16 state = encode_state(Pos);
    for (int i = 0; i < Moves.count; i++){
        capturePiece = make_move(Pos, Moves.moves[i]);
        
        // if (!(is_king_attacked(Pos))){
            nodes += run_perft(Pos, depth - 1);
        // }

        undo_move(Pos, Moves.moves[i], state, capturePiece);
    }

    return nodes;
}

// u64 perft(S_Position* Pos, uint depth){
//     u64 nodes = 0ULL, current_nodes;

//     S_Moves Moves;
//     S_Board Board_copy;
//     generateMoves(Pos, &Moves);
//     // memcpy(&Board_copy, Board, sizeof(S_Board));
//     printf("Nodes searched per move - depth: %u\n", depth);

//     u8 capturePiece;
//     u16 state = encode_state(Pos);
//     for (int i = 0; i < Moves.count; i++){
//         current_nodes = 0ULL;
//         capturePiece = make_move(Pos, Moves.moves[i]);
        
//         if (!(is_king_attacked(Pos))){
//             current_nodes += run_perft(Pos, depth - 1);
//         }

//         if (current_nodes != 0){
//             printf("%s%s",
//                 squares_int_to_chr[MOVE_FROM_SQUARE(Moves.moves[i])],
//                 squares_int_to_chr[MOVE_TO_SQUARE(Moves.moves[i])]
//             );
//             switch (MOVE_GET_FLAG(Moves.moves[i])) {
//                 case PROMOTION_B: printf("b"); break;
//                 case PROMOTION_R: printf("r"); break;
//                 case PROMOTION_N: printf("n"); break;
//                 case PROMOTION_Q: printf("q"); break;
//                 case CAPTURE_PROMOTION_B: printf("b"); break;
//                 case CAPTURE_PROMOTION_R: printf("r"); break;
//                 case CAPTURE_PROMOTION_N: printf("n"); break;
//                 case CAPTURE_PROMOTION_Q: printf("q"); break;
//                 default: break;
//             }
//             printf(": %"PRIu64"\n", current_nodes);
//         }

//         undo_move(Pos, Moves.moves[i], state, capturePiece);
//         // memcpy(Board, &Board_copy, sizeof(S_Board));
//         nodes += current_nodes;
//     }


//     return nodes;
// }

// u64 run_perft(S_Position* Pos, uint depth){
//     if (depth == 0 || is_king_attacked(Pos)) { return 1ULL; }

//     u64 nodes = 0ULL;
//     S_Moves Moves;
//     S_Board Board_copy;
//     generateMoves(Pos, &Moves);

//     u8 capturePiece;
//     const u16 state = encode_state(Pos);
//     for (int i = 0; i < Moves.count; i++){
//         capturePiece = make_move(Pos, Moves.moves[i]);
        
//         if (!(is_king_attacked(Pos))){
//             nodes += run_perft(Pos, depth - 1);
//         }

//         undo_move(Pos, Moves.moves[i], state, capturePiece);
//     }

//     return nodes;
// }