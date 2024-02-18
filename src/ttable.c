#include "defs.h"
#include <string.h>
#include <stdio.h>

/*
Source: https://www.chessprogramming.org/Zobrist_Hashing

If we now want to get the Zobrist hash code of a certain position, we initialize the hash key by xoring all random numbers linked to the given feature, e.g. the initial position:

[Hash for White Rook on a1] xor [Hash for White Knight on b1] xor [Hash for White Bishop on c1] xor ... ( all pieces )
... xor [Hash for White king castling] xor [Hash for White queeb castling] xor ... ( all castling rights )

The fact that xor-operation is own inverse and can be undone by using the same xor-operation again, is often used by chess engines. 
It allows a fast incremental update of the hash key during make or unmake moves. E.g., for a White Knight that jumps from b1 to c3 capturing a Black Bishop, these operations are performed:

[Original Hash of position] xor [Hash for White Knight on b1] ... ( removing the knight from b1 )
... xor [Hash for Black Bishop on c3] ( removing the captured bishop from c3 )
... xor [Hash for White Knight on c3] ( placing the knight on the new square )
... xor [Hash for Black to move] ( change sides)
*/

#define MB 1024
#define TT_SIZE 100 * MB

S_TTable TTable;
static int seed = 1804289383;

u64 TT_squares_hash[2][6][64];
u64 TT_castling_rights_hash[16];
u64 TT_enpassant_hash[8];
u64 TT_side_to_move_hash;

u32 get_random_u32(){
    u32 num = seed;
    num ^= (num << 13);
    num ^= (num >> 17);
    num ^= (num << 5);
    seed = num;
    return seed;
}

u64 get_pseudo_random_u64(){
    u64 n1 = (get_random_u32() & 0xFFFF);
    u64 n2 = (get_random_u32() & 0xFFFF);
    u64 n3 = (get_random_u32() & 0xFFFF);
    u64 n4 = (get_random_u32() & 0xFFFF);

    return n1 | (n2 << 16 ) | (n3 << 32) | (n4 << 48);
}

u64 get_random_u64(){
    return get_pseudo_random_u64() & get_pseudo_random_u64() & get_pseudo_random_u64();
}

void hash_position(S_Position* Pos){
    Pos->Board->hash = 0;
    u64 bb;
    u8 square;
    for (int piece = 0; piece < 6; piece++){
        bb = Pos->Board->piecesBB[piece] & Pos->Board->colorBB[BLACK];
        while (bb) {
            square = GET_LEAST_SIGNIFICANT_BIT_INDEX(bb);
            CLEAR_LEAST_SIGNIFICANT_BIT(bb);
            Pos->Board->hash ^= TT_squares_hash[BLACK][piece][square];
        }
        bb = Pos->Board->piecesBB[piece] & Pos->Board->colorBB[WHITE];
        while (bb) {
            square = GET_LEAST_SIGNIFICANT_BIT_INDEX(bb);
            CLEAR_LEAST_SIGNIFICANT_BIT(bb);
            Pos->Board->hash ^= TT_squares_hash[WHITE][piece][square];
        }
    }
    Pos->Board->hash ^= TT_castling_rights_hash[Pos->castlePermission];
    if (Pos->enPassantSquare)
        Pos->Board->hash ^= TT_enpassant_hash[Pos->enPassantSquare % 8];
    if (Pos->sideToMove == BLACK)
        Pos->Board->hash ^= TT_side_to_move_hash;
}

void init_TT(){
    // u64 testTable[(2 * 6 * 64) + 16 + 8 + 2];
    for (int side = 0; side < 2; side++){
        for (int piece = 0; piece < 6; piece++){
            for (int square = 0; square < 64; square++){
                TT_squares_hash[side][piece][square] = get_random_u64();
                // testTable[(side * piece) + square - 1] = TT_squares_hash[side][piece][square];
            }
        }
    }
    for (int i = 0; i < 16; i++){
        TT_castling_rights_hash[i] = get_random_u64();
        // testTable[(2 * 6 * 64) + i - 1] = TT_castling_rights_hash[i];
    }
    for (int i = 0; i < 8; i++){
        TT_enpassant_hash[i] = get_random_u64();
        // testTable[(2 * 6 * 64) + 16 + i - 1] = TT_castling_rights_hash[i];
    }
    TT_side_to_move_hash = get_random_u64();
    // testTable[(2 * 6 * 64) + 16 + 8 + 1 - 1] = TT_side_to_move_hash;

    TTable.count = TT_SIZE / sizeof(S_TT_Entry);
    TTable.entries = malloc(sizeof(S_TT_Entry) * TTable.count);
    memset(TTable.entries, 0, sizeof(S_TT_Entry) * TTable.count);


    // for (int side = 0; side < 2; side++){
    //     for (int piece = 0; piece < 6; piece++){
    //         for (int square = 0; square < 64; square++){
    //             u64 test = TT_squares_hash[side][piece][square];
    //             int i = piece, j = square;

    //             for (int _piece = 0; _piece < 12; _piece++){
    //                 for (int _square = 0; _square < 64; _square++){
    //                     if (_piece == i || _square == j)
    //                         continue;
    //                     if (TT_squares_hash[side][_piece][_square] == test)
    //                         printf("i1: %i, j1: %i | i2: %i, j2: %i | entry1: %llu, entry2: %llu\n", piece, square, _piece, _square, test, TT_squares_hash[side][_piece][_square]);
    //                 }
    //             }

    //             for (int i = 0; i < 16; i++){
    //                 if (TT_castling_rights_hash[i] == test)
    //                         printf("Castling rights hash collision\n");
    //             }
    //         }
    //     }
    // }

    // u64 uniqueCollsionsTable[(2 * 6 * 64) + 16 + 8 + 2];
    // int lastUnique = 0;
    // memset(uniqueCollsionsTable, 0, sizeof(u64) * (2 * 6 * 64) + 16 + 8 + 2);

    // int count = 0;
    // for (int i = 0; i < (2 * 6 * 64) + 16 + 8 + 2; i++){
    //     for (int j = 0; j < (2 * 6 * 64) + 16 + 8 + 2; j++){
    //         if (i == j)
    //             continue;
    //         if (testTable[i] == 0 || testTable[j] == 0)
    //             continue;
    //         if (testTable[i] == testTable[j]){
    //             for (int u = 0; u < (2 * 6 * 64) + 16 + 8 + 2; u++){
    //                 if (testTable[i] == uniqueCollsionsTable[u]){
    //                     uniqueCollsionsTable[lastUnique++] = testTable[i];
    //                 }
    //             } 
    //             // printf("collision!\n"); 
    //             count++;
    //         }
    //     }
    // }
    // printf("Veryfication ended, collisions: %i out of %i, other count? %i\n", lastUnique, (2 * 6 * 64) + 16 + 8 + 2, count);
}

