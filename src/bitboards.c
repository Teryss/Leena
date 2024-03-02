#include "defs.h"

u64 between[64][64];
u64 line[64][64];
u64 sqrs[64];

void init_bb(){
    for (int sq1 = 0; sq1 < 64; sq1++){
        sqrs[sq1] = 1ULL << sq1;
        for (int sq2 = 0; sq2 < 64; sq2++){

            if (get_bishop_attacks(0, sq1) & sqrs(sq2)){
                between[sq1][sq2] |= get_bishop_attacks(sqrs(sq2), sq1) & 
                                            get_bishop_attacks(sqrs(sq1), sq2);
                line[sq1][sq2] = (get_bishop_attacks(0, sq1) & get_bishop_attacks(0, sq2)) |
                                    sqrs(sq1) | sqrs(sq2);
            }
            if (get_rook_attacks(0, sq1) & sqrs(sq2)){
                between[sq1][sq2] |= get_rook_attacks(sqrs(sq2), sq1) & 
                                            get_rook_attacks(sqrs(sq1), sq2);
                line[sq1][sq2] = (get_rook_attacks(0, sq1) & get_rook_attacks(0, sq2)) |
                                    sqrs(sq1) | sqrs(sq2);
            }
        }
    }
}