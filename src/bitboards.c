#include "defs.h"

u64 sqrs[64];
u64 between[64][64];
u64 line[64][64];
// u64 ranks[8];
// u64 files[8];

void init_bb(){
    // for (int i = 0; i < 8; i++){
    //     ranks[i] = 0xFFULL << (u64)i * 8;
    //     files[i] = 0x101010101010101ULL << (u64)i;
    // }
    for (int i = 0; i < 64; i++){
        sqrs[i] = 1ULL << i;
    }
    for (int sq1 = 0; sq1 < 64; sq1++){
        for (int sq2 = 0; sq2 < 64; sq2++){
            between[sq1][sq2] = sqrs[sq2];

            if (get_bishop_attacks(0, sq1) & sqrs[sq2]){
                between[sq1][sq2] |= get_bishop_attacks(sqrs[sq2], sq1) & 
                                            get_bishop_attacks(sqrs[sq1], sq2);
                line[sq1][sq2] = (get_bishop_attacks(0, sq1) & get_bishop_attacks(0, sq2)) |
                                    sqrs[sq1] | sqrs[sq2];
            }
            if (get_rook_attacks(0, sq1) & sqrs[sq2]){
                between[sq1][sq2] |= get_rook_attacks(sqrs[sq2], sq1) & 
                                            get_rook_attacks(sqrs[sq1], sq2);
                line[sq1][sq2] = (get_rook_attacks(0, sq1) & get_rook_attacks(0, sq2)) |
                                    sqrs[sq1] | sqrs[sq2];
            }
        }
    }
}