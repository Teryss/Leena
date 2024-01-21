#include "defs.h"
#include <stdio.h>
#include <string.h>

#define CAPTURE_BONUS 10000
#define FIRST_KILLER_MOVE_BONUS 9000
#define SECOND_KILLER_MOVE_BONUS 8000

u32 killer_moves[MAX_GAME_SIZE][2];
const uint MVV_LVA[12][12] = {
    {105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605},
    {104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604},
    {103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603},
    {102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602},
    {101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601},
    {100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600},

    {105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605},
    {104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604},
    {103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603},
    {102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602},
    {101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601},
    {100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600},
};

const int16_t PIECE_VALUE[6] = {
    100, 350, 350, 525, 1000, 10000
};

const int16_t PAWN_SCORE[64] = {
    90,  90,  90,  90,  90,  90,  90,  90,
    30,  30,  30,  40,  40,  30,  30,  30,
    20,  20,  20,  30,  30,  30,  20,  20,
    10,  10,  10,  20,  20,  10,  10,  10,
    5,   5,  10,  20,  20,   5,   5,   5,
    0,   0,   0,   5,   5,   0,   0,   0,
    0,   0,   0, -10, -10,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
};

const int16_t KNIGHT_SCORE[64] = {
    -5,   0,   0,   0,   0,   0,   0,  -5,
    -5,   0,   0,  10,  10,   0,   0,  -5,
    -5,   5,  20,  20,  20,  20,   5,  -5,
    -5,  10,  20,  30,  30,  20,  10,  -5,
    -5,  10,  20,  30,  30,  20,  10,  -5,
    -5,   5,  20,  10,  10,  20,   5,  -5,
    -5,   0,   0,   0,   0,   0,   0,  -5,
    -5, -10,   0,   0,   0,   0, -10,  -5,
};

const int16_t BISHOP_SCORE[64] = {
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,  20,   0,  10,  10,   0,  20,   0,
    0,   0,  10,  20,  20,  10,   0,   0,
    0,   0,  10,  20,  20,  10,   0,   0,
    0,  10,   0,   0,   0,   0,  10,   0,
    0,  30,   0,   0,   0,   0,  30,   0,
    0,   0, -10,   0,   0, -10,   0,   0,
};

const int16_t ROOK_SCORE[64] = {
    50,  50,  50,  50,  50,  50,  50,  50,
    50,  50,  50,  50,  50,  50,  50,  50,
    0,   0,  10,  20,  20,  10,   0,   0,
    0,   0,  10,  20,  20,  10,   0,   0,
    0,   0,  10,  20,  20,  10,   0,   0,
    0,   0,  10,  20,  20,  10,   0,   0,
    0,   0,  10,  20,  20,  10,   0,   0,
    0,   0,   0,  20,  20,   0,   0,   0,
};

const int16_t QUEEN_SCORE[64] = {
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
};

const int16_t KING_SCORE[64] = {
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   5,   5,   5,   5,   0,   0,
    0,   5,   5,  10,  10,   5,   5,   0,
    0,   5,  10,  20,  20,  10,   5,   0,
    0,   5,  10,  20,  20,  10,   5,   0,
    0,   0,   5,  10,  10,   5,   0,   0,
    0,   5,   5,  -5,  -5,   0,   5,   0,
    0,   0,   5,   0, -15,   0,  10,   0,
};

const int16_t* PIECE_SQUARE_BONUS[6] = {
    PAWN_SCORE, KNIGHT_SCORE, BISHOP_SCORE, ROOK_SCORE, QUEEN_SCORE, KING_SCORE
};

const u8 mirror_square[64] = {
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8,
};

void clear_killer_moves(){
    memset(killer_moves, 0, sizeof(u32) * MAX_GAME_SIZE * 2);
}

i32 evaluate(S_Position* Pos){
    u64 bb;
    i32 eval = 0;
    const int8_t side_multiplier = Pos->sideToMove == BLACK ? -1 : 1;
    u8 square;

    for (int piece = 0; piece < 6; piece++){
        bb = Pos->Board->piecesBB[piece] & Pos->Board->colorBB[BLACK];
        while (bb) {
            square = GET_LEAST_SIGNIFICANT_BIT_INDEX(bb);
            CLEAR_LEAST_SIGNIFICANT_BIT(bb);
            eval -= PIECE_VALUE[piece];
            eval -= PIECE_SQUARE_BONUS[piece][mirror_square[square]];
        }

        bb = Pos->Board->piecesBB[piece] & Pos->Board->colorBB[WHITE];
        while (bb) {
            square = GET_LEAST_SIGNIFICANT_BIT_INDEX(bb);
            CLEAR_LEAST_SIGNIFICANT_BIT(bb);
            eval += PIECE_VALUE[piece - 6];
            eval += PIECE_SQUARE_BONUS[piece - 6][square];
        }
    }
    // for (int piece = 6; piece < 12; piece++){
    //     bb = Pos->Board->piecesBB[piece];
    //     while (bb) {
    //         square = GET_LEAST_SIGNIFICANT_BIT_INDEX(bb);
    //         CLEAR_LEAST_SIGNIFICANT_BIT(bb);
    //         eval += PIECE_VALUE[piece - 6];
    //         eval += PIECE_SQUARE_BONUS[piece - 6][square];
    //     }
    // }
    return eval * side_multiplier;
}

// void sort_moves(S_Board* Board, S_Moves* Moves, uint ply){
//     u64 temp_move;
//     uint scores[256];
//     uint temp_score;
//     for (int i = 0; i < Moves->count; i++){
//         if (MOVE_GET_FLAG(Moves->moves[i]) >= CAPTURE){
//             scores[i] = MVV_LVA[MOVE_GET_PIECE(Moves->moves[i])][MOVE_GET_CAPTURE_PIECE(Moves->moves[i])] + CAPTURE_BONUS;
//         }else{
//             if (Moves->moves[i] == killer_moves[0][ply]){
//                 scores[i] = FIRST_KILLER_MOVE_BONUS;
//             }else if (Moves->moves[i] == killer_moves[1][ply]){
//                 scores[i] = SECOND_KILLER_MOVE_BONUS;
//             }else{
//                 scores[i] = 0;
//             }    
//         }
//     }
    
//     for (int i = 0; i < Moves->count; i++){
//         if (scores[i] == 0)
//             continue;
//         for (int j = 0; j < Moves->count; j++){
//             if (scores[i] > scores[j]){
//                 temp_move = Moves->moves[j];
//                 Moves->moves[j] = Moves->moves[i];
//                 Moves->moves[i] = temp_move;

//                 temp_score = scores[j];
//                 scores[j] = scores[i];
//                 scores[i] = temp_score;
//             }
//         }
//     }
// }

// void sort_captures(S_Board* Board, S_Moves* Moves){
//     u64 temp_move;
//     uint scores[256];
//     uint temp_score;

//     for (int i = 0; i < Moves->count; i++){
//         scores[i] = MVV_LVA[MOVE_GET_PIECE(Moves->moves[i])][MOVE_GET_CAPTURE_PIECE(Moves->moves[i])];
//     }
//     for (int i = 0; i < Moves->count; i++){
//         for (int j = 0; j < Moves->count; j++){
//             if (scores[i] > scores[j]){
//                 temp_move = Moves->moves[j];
//                 Moves->moves[j] = Moves->moves[i];
//                 Moves->moves[i] = temp_move;

//                 temp_score = scores[j];
//                 scores[j] = scores[i];
//                 scores[i] = temp_score;
//             }
//         }
//     }
// }