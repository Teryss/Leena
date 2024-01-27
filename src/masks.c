#include "defs.h"

#define NOT_ON_A_FILE (18374403900871474942ULL)
#define NOT_ON_A_B_FILE (18229723555195321596ULL)
#define NOT_ON_G_H_FILE (4557430888798830399ULL)
#define NOT_ON_H_FILE (9187201950435737471ULL)
#define NOT_ON_8_RANK (18446744073709551360ULL)
#define NOT_ON_7_8_RANK (18446744073709486080ULL)
#define NOT_ON_1_RANK (72057594037927935ULL)
#define NOT_ON_1_2_RANK (281474976710655ULL)

#define ON_RANK_2 (0b11111111LLU << 8)
#define ON_RANK_7 (0b11111111LLU << 48)

#define UP -8
#define DOWN 8
#define RIGHT 1
#define LEFT -1

#define UP_RIGHT (UP + RIGHT)
#define UP_LEFT (UP + LEFT)
#define DOWN_RIGHT (DOWN + RIGHT)
#define DOWN_LEFT (DOWN + LEFT)

#define UP2_RIGHT (2 * UP + RIGHT)
#define UP2_LEFT (2 * UP + LEFT)
#define DOWN2_RIGHT (2 * DOWN + RIGHT)
#define DOWN2_LEFT (2 * DOWN + LEFT)
#define RIGHT2_UP (2 * RIGHT + UP)
#define RIGHT2_DOWN (2 * RIGHT + DOWN)
#define LEFT2_UP (2 * LEFT + UP)
#define LEFT2_DOWN (2 * LEFT + DOWN)

// max number of possible variotions - 4096
const uint ROOK_RELEVANT_BITS_BY_SQUARE[64] = {
    12, 11, 11, 11, 11, 11, 11, 12, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    12, 11, 11, 11, 11, 11, 11, 12,
};

// max number of possible variotions - 512
const uint BISHOP_RELEVANT_BITS_BY_SQUARE[64] = {
    6, 5, 5, 5, 5, 5, 5, 6, 
    5, 5, 5, 5, 5, 5, 5, 5, 
    5, 5, 7, 7, 7, 7, 5, 5, 
    5, 5, 7, 9, 9, 7, 5, 5, 
    5, 5, 7, 9, 9, 7, 5, 5, 
    5, 5, 7, 7, 7, 7, 5, 5, 
    5, 5, 5, 5, 5, 5, 5, 5, 
    6, 5, 5, 5, 5, 5, 5, 6,
};

/*
for (uint i = 0; i < 64; i++){
    if (i % 8 == 0) { printf("\n"); }
    printf("%i, ", start);
    start += (1ULL << BISHOP_RELEVANT_BITS_BY_SQUARE[i]);
    if (i == 63){ printf("end: %i\n", start - 1); }
}
*/
const uint BISHOP_PEXT_OFFSET[64] = {
    0, 64, 96, 128, 160, 192, 224, 256,
    320, 352, 384, 416, 448, 480, 512, 544,
    576, 608, 640, 768, 896, 1024, 1152, 1184,
    1216, 1248, 1280, 1408, 1920, 2432, 2560, 2592,
    2624, 2656, 2688, 2816, 3328, 3840, 3968, 4000,
    4032, 4064, 4096, 4224, 4352, 4480, 4608, 4640,
    4672, 4704, 4736, 4768, 4800, 4832, 4864, 4896,
    4928, 4992, 5024, 5056, 5088, 5120, 5152, 5184,
};

/*
for (uint i = 0; i < 64; i++){
    if (i % 8 == 0) { printf("\n"); }
    printf("%i, ", start);
    start += (1ULL << ROOK_RELEVANT_BITS_BY_SQUARE[i]);
    if (i == 63){ printf("end: %i\n", start - 1); }
}
*/
const uint ROOK_PEXT_OFFSET[64] = {
    5248, 9344, 11392, 13440, 15488, 17536, 19584, 21632,
    25728, 27776, 28800, 29824, 30848, 31872, 32896, 33920,
    35968, 38016, 39040, 40064, 41088, 42112, 43136, 44160,
    46208, 48256, 49280, 50304, 51328, 52352, 53376, 54400,
    56448, 58496, 59520, 60544, 61568, 62592, 63616, 64640,
    66688, 68736, 69760, 70784, 71808, 72832, 73856, 74880,
    76928, 78976, 80000, 81024, 82048, 83072, 84096, 85120,
    87168, 91264, 93312, 95360, 97408, 99456, 101504, 103552,
};

void init_masks(){
    for (int sqr = 0; sqr < 64; sqr++){
        Masks.pawn_attacks[WHITE][sqr] = mask_pawn_attacks(sqr, WHITE);
        Masks.pawn_attacks[BLACK][sqr] = mask_pawn_attacks(sqr, BLACK);
        
        Masks.king[sqr] = mask_king_attacks(sqr);
        Masks.knight[sqr] = mask_knight_attacks(sqr);
        Masks.bishop[sqr] = mask_bishop_attacks(sqr);
        Masks.rook[sqr] = mask_rook_attacks(sqr);
    }

    u64 blocker;
    int all_blocker_posibilities, pext_result;
    for (int sqr = 0; sqr < 64; sqr++){
        all_blocker_posibilities = (1ULL << BISHOP_RELEVANT_BITS_BY_SQUARE[sqr]);
        for (int i = 0; i < all_blocker_posibilities; i++){
            blocker = get_blockers(i, BISHOP_RELEVANT_BITS_BY_SQUARE[sqr], Masks.bishop[sqr]);
            pext_result = PEXT(blocker, Masks.bishop[sqr]);
            Masks.attacks[BISHOP_PEXT_OFFSET[sqr] + pext_result] = mask_bishop_attacks_on_the_fly(sqr, blocker);
        }
        all_blocker_posibilities = (1ULL << ROOK_RELEVANT_BITS_BY_SQUARE[sqr]);
        for (int i = 0; i < all_blocker_posibilities; i++){
            blocker = get_blockers(i, ROOK_RELEVANT_BITS_BY_SQUARE[sqr], Masks.rook[sqr]);
            pext_result = PEXT(blocker, Masks.rook[sqr]);
            Masks.attacks[ROOK_PEXT_OFFSET[sqr] + pext_result] = mask_rook_attacks_on_the_fly(sqr, blocker);
        }
    }
}

u64 get_blockers(uint index, uint relevant_bits, u64 mask){
    u64 blockers = 0;
    uint square = 0;
    for (uint i = 0; i < relevant_bits; i++){
        square = GET_LEAST_SIGNIFICANT_BIT_INDEX(mask);
        CLEAR_LEAST_SIGNIFICANT_BIT(mask);
        if ((index & (1 << i)) != 0){
            blockers |= (1ULL << square);
        }
    }
    return blockers;
}

u64 mask_pawn_attacks(int square, int color){
    u64 mask = 0;
    if (color == WHITE && GET_BIT(NOT_ON_8_RANK, square)){
        if (GET_BIT(NOT_ON_A_FILE, square)) { SET_BIT(mask, (square + UP_LEFT)); }
        if (GET_BIT(NOT_ON_H_FILE, square)) { SET_BIT(mask, (square + UP_RIGHT)); }
    }else if (color == BLACK && GET_BIT(NOT_ON_1_RANK, square)){
        if (GET_BIT(NOT_ON_H_FILE, square)) { SET_BIT(mask, (square + DOWN_RIGHT)); }
        if (GET_BIT(NOT_ON_A_FILE, square)) { SET_BIT(mask, (square + DOWN_LEFT)); }
    }
    return mask;
}

u64 mask_king_attacks(int square){
    u64 mask = 0;
    if (GET_BIT(NOT_ON_1_RANK, square)){
        SET_BIT(mask, (square + DOWN));
        if (GET_BIT(NOT_ON_A_FILE, square)){
            SET_BIT(mask, (square + LEFT));
            SET_BIT(mask, (square + DOWN_LEFT));
        }
        if(GET_BIT(NOT_ON_H_FILE, square)){
            SET_BIT(mask, (square + RIGHT));
            SET_BIT(mask, (square + DOWN_RIGHT));

        }
    }
    if(GET_BIT(NOT_ON_8_RANK, square)){
        SET_BIT(mask, (square + UP));
        if (GET_BIT(NOT_ON_A_FILE, square)){
            SET_BIT(mask, (square + LEFT));
            SET_BIT(mask, (square + UP_LEFT));
        }
        if(GET_BIT(NOT_ON_H_FILE, square)){
            SET_BIT(mask, (square + RIGHT));
            SET_BIT(mask, (square + UP_RIGHT));
        }
    }
    return mask;
}

u64 mask_knight_attacks(int square){
    u64 mask = 0;
    if(GET_BIT(NOT_ON_A_B_FILE, square)){
        if(GET_BIT(NOT_ON_1_RANK, square)){
            SET_BIT(mask, (square + LEFT2_DOWN));
        }
        if (GET_BIT(NOT_ON_8_RANK, square)){
            SET_BIT(mask, (square + LEFT2_UP));
        }
    }
    if(GET_BIT(NOT_ON_G_H_FILE, square)){
        if(GET_BIT(NOT_ON_1_RANK, square)){
            SET_BIT(mask, (square + RIGHT2_DOWN));
        }
        if (GET_BIT(NOT_ON_8_RANK, square)){
            SET_BIT(mask, (square + RIGHT2_UP));
        }
    }
    if (GET_BIT(NOT_ON_7_8_RANK, square)){
        if (GET_BIT(NOT_ON_A_FILE, square)){
            SET_BIT(mask, (square + UP2_LEFT));
        }
        if (GET_BIT(NOT_ON_H_FILE, square)){
            SET_BIT(mask, (square + UP2_RIGHT));
        }
    }
    if (GET_BIT(NOT_ON_1_2_RANK, square)){
        if (GET_BIT(NOT_ON_A_FILE, square)){
            SET_BIT(mask, (square + DOWN2_LEFT));
        }
        if (GET_BIT(NOT_ON_H_FILE, square)){
            SET_BIT(mask, (square + DOWN2_RIGHT));
        }
    }
    return mask;
}

u64 mask_rook_attacks(int square){
    u64 mask = 0;
    uint current_row = square / 8;
    uint current_col = square % 8; 

    for (int r = current_row + 1; r < 7; r++){
        SET_BIT(mask, ROW_COL_TO_SQR(r, current_col));
    }
    for (int r = current_row - 1; r > 0; r--){
        SET_BIT(mask, ROW_COL_TO_SQR(r, current_col));
    }
    for (int c = current_col - 1; c > 0; c--){
        SET_BIT(mask, ROW_COL_TO_SQR(current_row, c));
    }
    for (int c = current_col + 1; c < 7; c++){
        SET_BIT(mask, ROW_COL_TO_SQR(current_row, c));
    }
    
    return mask;
}

u64 mask_bishop_attacks(int square){
    u64 mask = 0;
    uint current_row = square / 8;
    uint current_col = square % 8;

    int r, c;
    for (r = current_row + 1, c = current_col + 1; r < 7 && c < 7; r++, c++){
        SET_BIT(mask, ROW_COL_TO_SQR(r, c));
    }
    for (r = current_row - 1, c = current_col + 1; r > 0 && c < 7; r--, c++){
        SET_BIT(mask, ROW_COL_TO_SQR(r, c));
    }
    for (r = current_row - 1, c = current_col - 1; r > 0 && c > 0; r--, c--){
        SET_BIT(mask, ROW_COL_TO_SQR(r, c));
    }
    for (r = current_row + 1, c = current_col - 1; r < 7 && c > 0; r++, c--){
        SET_BIT(mask, ROW_COL_TO_SQR(r, c));
    }

    return mask;
}

u64 mask_queen_attacks(int square){
    return (mask_bishop_attacks(square) | mask_rook_attacks(square)); 
}

u64 mask_rook_attacks_on_the_fly(int square, u64 blockers){
    u64 mask = 0;
    uint current_row = square / 8;
    uint current_col = square % 8; 

    for (int r = current_row + 1; r < 8; r++){
        SET_BIT(mask, ROW_COL_TO_SQR(r, current_col));
        if (GET_BIT(blockers, ROW_COL_TO_SQR(r, current_col))) { break; }
    }
    for (int r = current_row - 1; r > -1; r--){
        SET_BIT(mask, ROW_COL_TO_SQR(r, current_col));
        if (GET_BIT(blockers, ROW_COL_TO_SQR(r, current_col))) { break; }
    }
    for (int c = current_col - 1; c > -1; c--){
        SET_BIT(mask, ROW_COL_TO_SQR(current_row, c));
        if (GET_BIT(blockers, ROW_COL_TO_SQR(current_row, c))) { break; }
    }
    for (int c = current_col + 1; c < 8; c++){
        SET_BIT(mask, ROW_COL_TO_SQR(current_row, c));
        if (GET_BIT(blockers, ROW_COL_TO_SQR(current_row, c))) { break; }
    }
    
    return mask;
}

u64 mask_bishop_attacks_on_the_fly(int square, u64 blockers){
    u64 mask = 0;
    uint current_row = square / 8;
    uint current_col = square % 8;

    int r, c;
    for (r = current_row + 1, c = current_col + 1; r < 8 && c < 8; r++, c++){
        SET_BIT(mask, ROW_COL_TO_SQR(r, c));
        if (GET_BIT(blockers, ROW_COL_TO_SQR(r, c))) { break; }
    }
    for (r = current_row - 1, c = current_col + 1; r > -1 && c < 8; r--, c++){
        SET_BIT(mask, ROW_COL_TO_SQR(r, c));
        if (GET_BIT(blockers, ROW_COL_TO_SQR(r, c))) { break; }
    }
    for (r = current_row - 1, c = current_col - 1; r > -1 && c > -1; r--, c--){
        SET_BIT(mask, ROW_COL_TO_SQR(r, c));
        if (GET_BIT(blockers, ROW_COL_TO_SQR(r, c))) { break; }
    }
    for (r = current_row + 1, c = current_col - 1; r < 8 && c > -1; r++, c--){
        SET_BIT(mask, ROW_COL_TO_SQR(r, c));
        if (GET_BIT(blockers, ROW_COL_TO_SQR(r, c))) { break; }
    }

    return mask;
}