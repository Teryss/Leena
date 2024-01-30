#include <stdio.h>
#include <string.h>
#include "defs.h"

#define RANK_1 0xFFLLU

/* MOVE ENCODING - 16 bits

    0000 0000 0000 0000 0000 0000 0011 1111 - from square 
    0000 0000 0000 0000 0000 1111 1100 0000 - to square
    0000 0000 0000 0000 1111 0000 0000 0000 - move flag

*/

#define us() (Pos->Board->colorBB[Pos->sideToMove])
#define enemy() (Pos->Board->colorBB[1 - Pos->sideToMove])

INLINE CONST u16 encode_move(u8 from_square, u8 to_square, u8 move_flag){
    return (u16)(
        from_square |
        to_square << 6 |
        move_flag << 12
    );    
}

static INLINE u64 square_attackers(const S_Position* const Pos, const uint square){
    return (u64)(
        (get_rook_attacks(Pos->Board->colorBB[BOTH], square) & (Pos->Board->piecesBB[r] | Pos->Board->piecesBB[q]) & enemy()) |
        (get_bishop_attacks(Pos->Board->colorBB[BOTH], square) & (Pos->Board->piecesBB[b] | Pos->Board->piecesBB[q]) & enemy()) |
        (Masks.knight[square] & Pos->Board->piecesBB[n] & enemy()) |
        (Masks.pawn_attacks[Pos->sideToMove][square] & Pos->Board->piecesBB[p] & enemy())
    );
}

static INLINE u64 is_square_attacked_custom(const S_Position* const Pos, const u64 occ, const uint square){
    return (u64)(
        (get_rook_attacks(occ, square) & (Pos->Board->piecesBB[r] | Pos->Board->piecesBB[q]) & enemy()) |
        (get_bishop_attacks(occ, square) & (Pos->Board->piecesBB[b] | Pos->Board->piecesBB[q]) & enemy()) |
        (Masks.knight[square] & Pos->Board->piecesBB[n] & enemy()) |
        (Masks.pawn_attacks[Pos->sideToMove][square] & Pos->Board->piecesBB[p] & enemy())
    );
}

static INLINE u64 get_attacks(u64 occupancy, u8 pieceType, u8 square){
    switch (pieceType) {
        case b:
            return get_bishop_attacks(occupancy, square);
        case r:
            return get_rook_attacks(occupancy, square);
        case q:
            return get_queen_attacks(occupancy, square);
        case n:
            return Masks.knight[square];
        case k:
            return Masks.king[square];
        default:
            return 0;
    }
}

static inline void _generate_all(const S_Position* const Pos, S_Moves* Moves, u64 empty_or_enemy, u8 pieceType){
    u64 bb = Pos->Board->piecesBB[pieceType] & us(), attack_bb;
    u8 from_square, to_square;

    while(bb){
        from_square = GET_LEAST_SIGNIFICANT_BIT_INDEX(bb);
        CLEAR_LEAST_SIGNIFICANT_BIT(bb);
        attack_bb = get_attacks(Pos->Board->colorBB[BOTH], pieceType, from_square) & empty_or_enemy;

        while(attack_bb){
            to_square = GET_LEAST_SIGNIFICANT_BIT_INDEX(attack_bb);
            CLEAR_LEAST_SIGNIFICANT_BIT(attack_bb);
            if (sqrs[to_square] & Pos->Board->colorBB[1 - Pos->sideToMove]){
                Moves->moves[Moves->count++] = encode_move(from_square, to_square, CAPTURE);
            }else{
                Moves->moves[Moves->count++] = encode_move(from_square, to_square, QUIET);
            }
        }
    }
}

static inline void _generate_captures(const S_Position* const Pos, S_Moves* Moves, u8 pieceType){
    u64 bb = Pos->Board->piecesBB[pieceType] & us(), attack_bb;
    u8 from_square, to_square;

    while(bb){
        from_square = GET_LEAST_SIGNIFICANT_BIT_INDEX(bb);
        CLEAR_LEAST_SIGNIFICANT_BIT(bb);
        attack_bb = get_attacks(Pos->Board->colorBB[BOTH], pieceType, from_square) & enemy();

        while(attack_bb){
            to_square = GET_LEAST_SIGNIFICANT_BIT_INDEX(attack_bb);
            CLEAR_LEAST_SIGNIFICANT_BIT(attack_bb);
            Moves->moves[Moves->count++] = encode_move(from_square, to_square, CAPTURE);
        }
    }
}

void generateOnlyCaptures(const S_Position* const Pos, S_Moves* Moves){
    const uint enemy_color = 1 - Pos->sideToMove;
    uint from_square, to_sqr;
    u64 bb, attack_bb = 0;
    Moves->count = 0;

    bb = Pos->Board->piecesBB[p] & us();
    if (Pos->enPassantSquare){
        u64 ep_bb = Masks.pawn_attacks[enemy_color][Pos->enPassantSquare] & bb;
        if (ep_bb){
            if (MORE_THAN_ONE(ep_bb)){
                for (uint i = 0; i < 2; i++){
                    from_square = GET_LEAST_SIGNIFICANT_BIT_INDEX(ep_bb);
                    CLEAR_LEAST_SIGNIFICANT_BIT(ep_bb);
                    Moves->moves[Moves->count++] = encode_move(from_square, Pos->enPassantSquare, EP_CAPTURE);
                }
            }else{
                Moves->moves[Moves->count++] = encode_move(GET_LEAST_SIGNIFICANT_BIT_INDEX(ep_bb), Pos->enPassantSquare, EP_CAPTURE);
            }
        }
    }

    while(bb){
        from_square = GET_LEAST_SIGNIFICANT_BIT_INDEX(bb);
        CLEAR_LEAST_SIGNIFICANT_BIT(bb);
        
        attack_bb = Masks.pawn_attacks[Pos->sideToMove][from_square] & enemy();
        while(attack_bb){
            to_sqr = GET_LEAST_SIGNIFICANT_BIT_INDEX(attack_bb);
            CLEAR_LEAST_SIGNIFICANT_BIT(attack_bb);
            if ((GET_BIT((RANK_1 << 8ULL), from_square) && Pos->sideToMove == WHITE) || (GET_BIT((RANK_1 << 48ULL), from_square) && Pos->sideToMove == BLACK)){
                Moves->moves[Moves->count++] = encode_move(from_square, to_sqr, CAPTURE_PROMOTION_R);
                Moves->moves[Moves->count++] = encode_move(from_square, to_sqr, CAPTURE_PROMOTION_Q);
                Moves->moves[Moves->count++] = encode_move(from_square, to_sqr, CAPTURE_PROMOTION_N);
                Moves->moves[Moves->count++] = encode_move(from_square, to_sqr, CAPTURE_PROMOTION_B);
            }else{
                Moves->moves[Moves->count++] = encode_move(from_square, to_sqr, CAPTURE);
            }
        }
    }

    _generate_captures(Pos, Moves, n);
    _generate_captures(Pos, Moves, r);
    _generate_captures(Pos, Moves, b);
    _generate_captures(Pos, Moves, q);

    bb = Pos->Board->piecesBB[k] & us();
    from_square = GET_LEAST_SIGNIFICANT_BIT_INDEX(bb);
    CLEAR_LEAST_SIGNIFICANT_BIT(bb);
    attack_bb = Masks.king[from_square] & enemy();
    while(attack_bb){
        to_sqr = GET_LEAST_SIGNIFICANT_BIT_INDEX(attack_bb);
        CLEAR_LEAST_SIGNIFICANT_BIT(attack_bb);
        Moves->moves[Moves->count++] = encode_move(from_square, to_sqr, CAPTURE);
    }
}

void generateMoves(const S_Position* const Pos, S_Moves* Moves){
    const u64 empty_or_enemy = ~Pos->Board->colorBB[Pos->sideToMove];
    u64 bb, attack_bb = 0;
    const u8 enemy_color = 1 - Pos->sideToMove;

    uint from_square, to_sqr;
    Moves->count = 0;

    bb = Pos->Board->piecesBB[p] & us();
    if (Pos->enPassantSquare){
        u64 ep_bb = Masks.pawn_attacks[enemy_color][Pos->enPassantSquare] & bb;
        if (ep_bb){
            if (MORE_THAN_ONE(ep_bb)){
                for (uint i = 0; i < 2; i++){
                    from_square = GET_LEAST_SIGNIFICANT_BIT_INDEX(ep_bb);
                    CLEAR_LEAST_SIGNIFICANT_BIT(ep_bb);
                    Moves->moves[Moves->count++] = encode_move(from_square, Pos->enPassantSquare, EP_CAPTURE);
                }
            }else{
                Moves->moves[Moves->count++] = encode_move(GET_LEAST_SIGNIFICANT_BIT_INDEX(ep_bb), Pos->enPassantSquare, EP_CAPTURE);
            }

        }
    }
    while(bb){
        from_square = GET_LEAST_SIGNIFICANT_BIT_INDEX(bb);
        CLEAR_LEAST_SIGNIFICANT_BIT(bb);

        uint temp_square = from_square + 8 - 16 * enemy_color;
        if(!(Pos->Board->colorBB[BOTH] & (1ULL << temp_square))){
            if ((GET_BIT((RANK_1 << 8ULL), from_square) && Pos->sideToMove == WHITE) || (GET_BIT((RANK_1 << 48ULL), from_square) && Pos->sideToMove == BLACK)){
                Moves->moves[Moves->count++] = encode_move(from_square, temp_square, PROMOTION_R);
                Moves->moves[Moves->count++] = encode_move(from_square, temp_square, PROMOTION_Q);
                Moves->moves[Moves->count++] = encode_move(from_square, temp_square, PROMOTION_N);
                Moves->moves[Moves->count++] = encode_move(from_square, temp_square, PROMOTION_B);
            }else{
                Moves->moves[Moves->count++] = encode_move(from_square, temp_square, QUIET);

                temp_square += 8 - 16 * enemy_color;
                if((1ULL << from_square) & RANK_1 << (u64)(8 + (40 * enemy_color))){
                    if (!(Pos->Board->colorBB[BOTH] & (1ULL << temp_square))){
                        Moves->moves[Moves->count++] = encode_move(from_square, (from_square + 16 - 32 * enemy_color), DOUBLE_PUSH);
                    }
                }
            }
        }

        attack_bb = Masks.pawn_attacks[Pos->sideToMove][from_square] & enemy();
        while(attack_bb){
            to_sqr = GET_LEAST_SIGNIFICANT_BIT_INDEX(attack_bb);
            CLEAR_LEAST_SIGNIFICANT_BIT(attack_bb);
            if ((GET_BIT((RANK_1 << 8ULL), from_square) && Pos->sideToMove == WHITE) || (GET_BIT((RANK_1 << 48ULL), from_square) && Pos->sideToMove == BLACK)){
                Moves->moves[Moves->count++] = encode_move(from_square, to_sqr, CAPTURE_PROMOTION_R);
                Moves->moves[Moves->count++] = encode_move(from_square, to_sqr, CAPTURE_PROMOTION_Q);
                Moves->moves[Moves->count++] = encode_move(from_square, to_sqr, CAPTURE_PROMOTION_N);
                Moves->moves[Moves->count++] = encode_move(from_square, to_sqr, CAPTURE_PROMOTION_B);
            }else{
                Moves->moves[Moves->count++] = encode_move(from_square, to_sqr, CAPTURE);
            }
        }
    }

    _generate_all(Pos, Moves, empty_or_enemy, n);
    _generate_all(Pos, Moves, empty_or_enemy, b);
    _generate_all(Pos, Moves, empty_or_enemy, r);
    _generate_all(Pos, Moves, empty_or_enemy, q);

    bb = Pos->Board->piecesBB[k] & us();
    // if (!bb) return;
    from_square = GET_LEAST_SIGNIFICANT_BIT_INDEX(bb);
    CLEAR_LEAST_SIGNIFICANT_BIT(bb);
    attack_bb = (Masks.king[from_square] & empty_or_enemy);
    while(attack_bb){
        to_sqr = GET_LEAST_SIGNIFICANT_BIT_INDEX(attack_bb);
        CLEAR_LEAST_SIGNIFICANT_BIT(attack_bb);
        // if (square_attackers(board, to_sqr))
        //     continue;
        if (sqrs[to_sqr] & Pos->Board->colorBB[1 - Pos->sideToMove]){
            Moves->moves[Moves->count++] = encode_move(from_square, to_sqr, CAPTURE);
        }else{
            Moves->moves[Moves->count++] = encode_move(from_square, to_sqr, QUIET);
        }
    }

    #define SQUARES_NEEDED_EMPTY__CASTLE_KING 0x6000000000000000LLU
    #define SQUARES_NEEDED_EMPTY__CASTLE_QUEEN 0xE00000000000000LLU
    #define REFLECT_SQUARE_OFFSET 56

    if (Pos->castlePermission && !square_attackers(Pos, GET_LEAST_SIGNIFICANT_BIT_INDEX(Pos->Board->piecesBB[k] & us()))){
        if (((Pos->castlePermission & wk) * enemy_color) | ((Pos->castlePermission & bk) * (Pos->sideToMove))){
            if(!(Pos->Board->colorBB[BOTH] & (SQUARES_NEEDED_EMPTY__CASTLE_KING >> REFLECT_SQUARE_OFFSET * Pos->sideToMove))){
                if (square_attackers(Pos, (F1 - REFLECT_SQUARE_OFFSET * Pos->sideToMove)) == 0 && square_attackers(Pos, (G1 - REFLECT_SQUARE_OFFSET * Pos->sideToMove)) == 0){
                    Moves->moves[Moves->count++] = encode_move(from_square, G1 - REFLECT_SQUARE_OFFSET * Pos->sideToMove, KING_CASTLE);
                }
            }
        }
        if (((Pos->castlePermission & wq) * enemy_color) | ((Pos->castlePermission & bq) * (Pos->sideToMove))){
            if(!(Pos->Board->colorBB[BOTH] & (SQUARES_NEEDED_EMPTY__CASTLE_QUEEN >> REFLECT_SQUARE_OFFSET * Pos->sideToMove))){
                if (square_attackers(Pos, (D1 - REFLECT_SQUARE_OFFSET * Pos->sideToMove)) == 0 && square_attackers(Pos, (C1 - REFLECT_SQUARE_OFFSET * Pos->sideToMove)) == 0){ 
                    Moves->moves[Moves->count++] = encode_move(from_square, C1 - REFLECT_SQUARE_OFFSET * Pos->sideToMove, QUEEN_CASTLE);
                }
            }
        }
    }
}

u64 pins(const S_Position* const Pos, u8 king_square){
    u64 xRayAttackers = (
        (Pos->Board->piecesBB[r] | Pos->Board->piecesBB[q]) & us() & get_rook_attacks(0, king_square) | 
        (Pos->Board->piecesBB[b] | Pos->Board->piecesBB[q]) & us() & get_bishop_attacks(0, king_square)
    );
    u64 pins = 0ULL, possible_pins;
    u8 attacker_square;

    while (xRayAttackers) {
        attacker_square = GET_LEAST_SIGNIFICANT_BIT_INDEX(xRayAttackers);
        CLEAR_LEAST_SIGNIFICANT_BIT(xRayAttackers);
        
        possible_pins = between[king_square][attacker_square] & us();
        if (possible_pins && !(MORE_THAN_ONE(possible_pins))){
            pins |= possible_pins;
        }
    }
    return pins;
}

// u64 is_legal(const S_Position* Pos, u8 kingSquare, u32 Move, u64 _pin, u64 checkers) { 
//     // return 0;
//     u8 from_square = MOVE_GET_FROM_SQUARE(Move), to_square = MOVE_GET_TO_SQUARE(Move), pieceType = MOVE_GET_PIECE(Move);


//     /* if it's a double check, a king has to move */
//     if (MORE_THAN_ONE(checkers)){
//         if (from_square != kingSquare)
//             return 0;
        
//         if (is_square_attacked_custom(
//                 Pos,
//                 Pos->Board->colorBB[BOTH] ^ sqrs[kingSquare],
//                 to_square)){    
//             return 0;
//         }
//         return 1;
//     /* if it's a check we can:
//         King:
//             move a king
//             capture attacking piece if it isn't protected
//         Other:
//             block a check
//             capture attacking piece
//     */
//     }else if (checkers){
//         if (from_square == kingSquare){
//             // check if captures wit king work properly
//             if (is_square_attacked_custom(
//                     Pos,
//                     Pos->Board->colorBB[BOTH] ^ sqrs[kingSquare],
//                     to_square))  
//                 return 0;
    
//             return 1;
//         }else{
//             // is already pinned
//             if (sqrs[from_square] & _pin)
//                 return 0;
//             // blocks or captures
//             if (sqrs[to_square] & (between[kingSquare][GET_LEAST_SIGNIFICANT_BIT_INDEX(checkers)]))
//                 return 1;
//             return 0;
//         }

//     }


//     /* if there is no check, we only need to care about pins and en passant discover checks*/
//     if (MOVE_GET_FLAG(Move) == EP_CAPTURE)
//         // todo
//         return 0;

//     if (_pin & sqrs[from_square]){
//         return line[kingSquare][from_square] & sqrs[to_square];
//     }

//     return 1;
// };

// void filter_illegal(const S_Position* const Pos, S_Moves* Moves){
//     u8 king_square = GET_LEAST_SIGNIFICANT_BIT_INDEX(Pos->Board->piecesBB[k] & us());

//     const u16* end = Moves->moves + Moves->count;
//     const u64 _pins = pins(Pos, king_square);
//     const u64 isInCheck = square_attackers(Pos, king_square);

//     // printf("-- KS: %s ; isInCheck: %llu, pins:\n", squares_int_to_chr[king_square], isInCheck);
//     // print_bitboard(board->occupied_squares_by[BOTH] ^ sqrs[king_square], NO_SQR);

//     u16* lastMove = Moves->moves;

//     for (u16* currentMove = Moves->moves; currentMove != end; currentMove++){
//         if (!is_legal(Pos, king_square, *currentMove, _pins, isInCheck)){
//             Moves->count--;
//             continue;
//         }
//         *lastMove = *currentMove;
//         lastMove++;
//     }
// }


/*
It's used during search/perft, when sideToMove has already been changed
During move generation we use is_square_attacked and specify king's location
*/

// idk if us or enemy!
PURE u8 is_king_attacked(const S_Position* const Pos){
    const uint square = GET_LEAST_SIGNIFICANT_BIT_INDEX(Pos->Board->piecesBB[k] & enemy());
    if (get_rook_attacks(Pos->Board->colorBB[BOTH], square) & (Pos->Board->piecesBB[r] | Pos->Board->piecesBB[q]) & us()) return 1;
    if (get_bishop_attacks(Pos->Board->colorBB[BOTH], square) & (Pos->Board->piecesBB[b] | Pos->Board->piecesBB[q]) & us()) return 1;
    if (Masks.knight[square] & Pos->Board->piecesBB[n] & us()) return 1;
    if (Masks.pawn_attacks[1 - Pos->sideToMove][square] & Pos->Board->piecesBB[p] & us()) return 1;
    if (Masks.king[square] & Pos->Board->piecesBB[k] & us()) return 1;
    // if (Masks.pawn_attacks[us()][square] & Pos->Board->piecesBB[p] & us()) return 1;
    return 0;
}