#include <string.h>
#include <stdio.h>
#include "defs.h"

#define RANK_1 0xFFLLU

/* STATE ENCODING - 16 bits, all used

    0000 0000 0011 1111 - en passant square
    0000 0011 1100 0000 - castle permission
    1111 1100 0000 0000 - fifty moves counter

*/

INLINE CONST u16 encode_state(const S_Board* const Board){
    return (u16)(
        Board->enPassantSquare |
        Board->castlePermission << 6UL |
        Board->fiftyMovesCounter << 10UL
    );
}

/* MOVE ENCODING - 32 bits, 28 required

    0000 0000 0000 0000 0000 0000 0000 1111 - piece to move 
    0000 0000 0000 0000 0000 0011 1111 0000 - from square
    0000 0000 0000 0000 1111 1100 0000 0000 - to square
    0000 0000 0000 1111 0000 0000 0000 0000 - promotion piece (redundant)
    0000 0000 1111 0000 0000 0000 0000 0000 - capture piece
    0000 1111 0000 0000 0000 0000 0000 0000 - move flag

*/

INLINE CONST u32 encode_move(u8 piece, u8 from_square, u8 to_square, u8 promotion_piece, u8 capture_piece, u8 move_flag){
    return (u32)(
        piece |
        from_square << 4UL |
        to_square << 10UL |
        promotion_piece << 16UL |
        capture_piece << 20UL |
        move_flag << 24UL
    );    
}

static inline PURE uint get_target_piece(const S_Board* const board, uint square){
    for (int piece = (6 * board->sideToMove); piece < 6 + (6 * board->sideToMove); piece++){
        if (GET_BIT(board->pieces[piece], square)){
            return piece;
        }
    }
    return GET_PIECE_FAILED_FLAG;
}

INLINE PURE u64 get_rook_attacks(u64 occupancy, const uint square){
    return Masks.attacks[PEXT(occupancy, Masks.rook[square]) + ROOK_PEXT_OFFSET[square]];
}

INLINE PURE u64 get_bishop_attacks(u64 occupancy, const uint square){
    return Masks.attacks[PEXT(occupancy, Masks.bishop[square]) + BISHOP_PEXT_OFFSET[square]];
}

INLINE PURE u64 get_queen_attacks(u64 occupancy, const uint square){
    return get_bishop_attacks(occupancy, square) | get_rook_attacks(occupancy, square);
}

static inline PURE u8 is_square_attacked(const S_Board* const board, const uint square){
    if (get_rook_attacks(board->occupied_squares_by[BOTH], square) & board->pieces[r + 6 * board->sideToMove]) return 1;
    if (get_bishop_attacks(board->occupied_squares_by[BOTH], square) & board->pieces[b + 6 * board->sideToMove]) return 1; 
    if (get_queen_attacks(board->occupied_squares_by[BOTH], square) & board->pieces[q + 6 * board->sideToMove]) return 1;
    if (Masks.knight[square] & board->pieces[n + 6 * board->sideToMove]) return 1;
    if (Masks.pawn_attacks[board->sideToMove][square]  & board->pieces[p + 6 * board->sideToMove]) return 1;
    return 0;
}

INLINE PURE u8 is_king_attacked(const S_Board* const board){
    const uint square = GET_LEAST_SIGNIFICANT_BIT_INDEX(board->pieces[k + 6 * board->sideToMove]);
        if (get_rook_attacks(board->occupied_squares_by[BOTH], square) & (board->pieces[R - 6 * board->sideToMove])) return 1;
        if (get_bishop_attacks(board->occupied_squares_by[BOTH], square) & board->pieces[B - 6 * board->sideToMove]) return 1; 
        if (get_queen_attacks(board->occupied_squares_by[BOTH], square) & board->pieces[Q - 6 * board->sideToMove]) return 1;
        if (Masks.pawn_attacks[board->sideToMove^1][square]  & board->pieces[P - 6 * board->sideToMove]) return 1;
        if (Masks.knight[square] & board->pieces[N - 6 * board->sideToMove]) return 1;
        if (Masks.king[square] & board->pieces[K - 6 * board->sideToMove]) return 1;
    return 0;
}

// u64 get_pins(const S_Board* const Board){
//     u64 possible_pins = 0ULL;
//     u64 pins = 0ULL;
//     const uint offset = 6 * Board->sideToMove, king_square = GET_LEAST_SIGNIFICANT_BIT_INDEX(Board->pieces[k + offset]);
    
    
//     // possible_pins |= get_queen_attacks(Board, king_square) & Board->pieces[q + offset];
    

//     return pins;
// }

// void generateLegalMoves(const S_Board* const board, S_Moves* Moves){
//     generateMoves(board, Moves);
//     S_Moves legal;
//     memcpy(&legal, Moves, sizeof(S_Moves));


// }

void generateMoves(const S_Board* const board, S_Moves* Moves){
    const uint side_to_move = board->sideToMove;
    const uint pieces_offset = side_to_move == WHITE ? 6 : 0;
    const uint enemy_color = 1 - side_to_move;
    uint from_square, to_sqr;
    const u64 empty_or_enemy = ~board->occupied_squares_by[side_to_move];
    u64 bb, attack_bb = 0;
    Moves->count = 0;

    bb = board->pieces[p + pieces_offset];
    while(bb){
        from_square = GET_LEAST_SIGNIFICANT_BIT_INDEX(bb);
        CLEAR_LEAST_SIGNIFICANT_BIT(bb);
        
        if (board->enPassantSquare != NO_SQR){
            if ((RANK_1 << 8ULL * (4 - 1 * enemy_color)) & (1ULL << from_square)){
                u64 enPassantAttack = Masks.pawn_attacks[side_to_move][from_square] & (1ULL << board->enPassantSquare);
                if (enPassantAttack){
                    Moves->moves[Moves->count++] = encode_move(p + pieces_offset, from_square, board->enPassantSquare, NO_PIECE, P - pieces_offset, EP_CAPTURE);
                }
            }
        }

        uint temp_square = from_square + 8 - 16 * enemy_color;
        if(!(board->occupied_squares_by[BOTH] & (1ULL << temp_square))){
            if (GET_BIT((RANK_1 << 8ULL), from_square) && side_to_move == WHITE || GET_BIT((RANK_1 << 48ULL), from_square) && side_to_move == BLACK){
                Moves->moves[Moves->count++] = encode_move(p + pieces_offset, from_square, temp_square, r + pieces_offset, NO_PIECE, PROMOTION_R);
                Moves->moves[Moves->count++] = encode_move(p + pieces_offset, from_square, temp_square, q + pieces_offset, NO_PIECE, PROMOTION_Q);
                Moves->moves[Moves->count++] = encode_move(p + pieces_offset, from_square, temp_square, n + pieces_offset, NO_PIECE, PROMOTION_N);
                Moves->moves[Moves->count++] = encode_move(p + pieces_offset, from_square, temp_square, b + pieces_offset, NO_PIECE, PROMOTION_B);
            }else{
                Moves->moves[Moves->count++] = encode_move(p + pieces_offset, from_square, temp_square, NO_PIECE, NO_PIECE, QUIET);

                temp_square = temp_square + 8 - 16 * enemy_color;
                if((1ULL << from_square) &  RANK_1 << (u64)(8 + (40 * enemy_color))){
                    if (!(board->occupied_squares_by[BOTH] & (1ULL << temp_square))){
                        Moves->moves[Moves->count++] = encode_move(p + pieces_offset, from_square, (from_square + 16 - 32 * enemy_color), NO_PIECE, NO_PIECE, DOUBLE_PUSH);
                    }
                }
            }
        }

        attack_bb = Masks.pawn_attacks[side_to_move][from_square] & board->occupied_squares_by[enemy_color];
        while(attack_bb){
            to_sqr = GET_LEAST_SIGNIFICANT_BIT_INDEX(attack_bb);
            CLEAR_LEAST_SIGNIFICANT_BIT(attack_bb);
            if (GET_BIT((RANK_1 << 8ULL), from_square) && side_to_move == WHITE || GET_BIT((RANK_1 << 48ULL), from_square) && side_to_move == BLACK){
                Moves->moves[Moves->count++] = encode_move(p + pieces_offset, from_square, to_sqr, r + pieces_offset, get_target_piece(board, to_sqr), CAPTURE_PROMOTION_R);
                Moves->moves[Moves->count++] = encode_move(p + pieces_offset, from_square, to_sqr, q + pieces_offset, get_target_piece(board, to_sqr), CAPTURE_PROMOTION_Q);
                Moves->moves[Moves->count++] = encode_move(p + pieces_offset, from_square, to_sqr, n + pieces_offset, get_target_piece(board, to_sqr), CAPTURE_PROMOTION_N);
                Moves->moves[Moves->count++] = encode_move(p + pieces_offset, from_square, to_sqr, b + pieces_offset, get_target_piece(board, to_sqr), CAPTURE_PROMOTION_B);
            }else{
                Moves->moves[Moves->count++] = encode_move(p + pieces_offset, from_square, to_sqr, NO_PIECE, get_target_piece(board, to_sqr), CAPTURE);
            }
        }
    }

    bb = board->pieces[k + pieces_offset];
    while(bb){
        from_square = GET_LEAST_SIGNIFICANT_BIT_INDEX(bb);
        CLEAR_LEAST_SIGNIFICANT_BIT(bb);
        attack_bb = (Masks.king[from_square] & empty_or_enemy);

        while(attack_bb){
            to_sqr = GET_LEAST_SIGNIFICANT_BIT_INDEX(attack_bb);
            CLEAR_LEAST_SIGNIFICANT_BIT(attack_bb);
            if ((1ULL << to_sqr) & ~(board->occupied_squares_by[BOTH])){
                Moves->moves[Moves->count++] = encode_move(k + pieces_offset, from_square, to_sqr, NO_PIECE, NO_PIECE, QUIET);
            }else{
                Moves->moves[Moves->count++] = encode_move(k + pieces_offset, from_square, to_sqr, NO_PIECE, get_target_piece(board, to_sqr), CAPTURE);
            }
        }

        #define SQUARES_NEEDED_EMPTY__CASTLE_KING 0x6000000000000000LLU
        #define SQUARES_NEEDED_EMPTY__CASTLE_QUEEN 0xE00000000000000LLU
        #define REFLECT_SQUARE_OFFSET 56

        if (board->castlePermission && !is_square_attacked(board, GET_LEAST_SIGNIFICANT_BIT_INDEX(board->pieces[k + pieces_offset]))){
            if (((board->castlePermission & wk) * enemy_color) | ((board->castlePermission & bk) * (side_to_move))){
                if(!(board->occupied_squares_by[BOTH] & (SQUARES_NEEDED_EMPTY__CASTLE_KING >> REFLECT_SQUARE_OFFSET * side_to_move))){
                    if (is_square_attacked(board, (F1 - REFLECT_SQUARE_OFFSET * side_to_move)) == 0){
                        Moves->moves[Moves->count++] = encode_move(k + pieces_offset, from_square, G1 - REFLECT_SQUARE_OFFSET * side_to_move, NO_PIECE, NO_PIECE, KING_CASTLE);
                    }
                }
            }
            if (((board->castlePermission & wq) * enemy_color) | ((board->castlePermission & bq) * (side_to_move))){
                if(!(board->occupied_squares_by[BOTH] & (SQUARES_NEEDED_EMPTY__CASTLE_QUEEN >> REFLECT_SQUARE_OFFSET * side_to_move))){
                    if (is_square_attacked(board, (D1 - REFLECT_SQUARE_OFFSET * side_to_move)) == 0){ 
                        Moves->moves[Moves->count++] = encode_move(k + pieces_offset, from_square, C1 - REFLECT_SQUARE_OFFSET * side_to_move, NO_PIECE, NO_PIECE, QUEEN_CASTLE);
                    }
                }
            }
        }
    }

    bb = board->pieces[n + pieces_offset];
    while(bb){
        from_square = GET_LEAST_SIGNIFICANT_BIT_INDEX(bb);
        CLEAR_LEAST_SIGNIFICANT_BIT(bb);
        attack_bb = (Masks.knight[from_square] & empty_or_enemy);

        while(attack_bb){
            to_sqr = GET_LEAST_SIGNIFICANT_BIT_INDEX(attack_bb);
            CLEAR_LEAST_SIGNIFICANT_BIT(attack_bb);
            if ((1ULL << to_sqr) & ~(board->occupied_squares_by[BOTH])){
                Moves->moves[Moves->count++] = encode_move(n + pieces_offset, from_square, to_sqr, NO_PIECE, NO_PIECE, QUIET);
            }else{
                Moves->moves[Moves->count++] = encode_move(n + pieces_offset, from_square, to_sqr, NO_PIECE, get_target_piece(board, to_sqr), CAPTURE);
            }
        }
    }

    bb = board->pieces[r + pieces_offset];
    while (bb) {
        from_square = GET_LEAST_SIGNIFICANT_BIT_INDEX(bb);
        CLEAR_LEAST_SIGNIFICANT_BIT(bb);
        attack_bb = get_rook_attacks(board->occupied_squares_by[BOTH], from_square) & empty_or_enemy;

        while(attack_bb){
            to_sqr = GET_LEAST_SIGNIFICANT_BIT_INDEX(attack_bb);
            CLEAR_LEAST_SIGNIFICANT_BIT(attack_bb);
            if ((1ULL << to_sqr) & ~(board->occupied_squares_by[BOTH])){
                Moves->moves[Moves->count++] = encode_move(r + pieces_offset, from_square, to_sqr, NO_PIECE, NO_PIECE, QUIET);
            }else{
                Moves->moves[Moves->count++] = encode_move(r + pieces_offset, from_square, to_sqr, NO_PIECE, get_target_piece(board, to_sqr), CAPTURE);
            }
        }
    }

    bb = board->pieces[b + pieces_offset];
    while (bb) {
        from_square = GET_LEAST_SIGNIFICANT_BIT_INDEX(bb);
        CLEAR_LEAST_SIGNIFICANT_BIT(bb);
        attack_bb = get_bishop_attacks(board->occupied_squares_by[BOTH], from_square) & empty_or_enemy;

        while(attack_bb){
            to_sqr = GET_LEAST_SIGNIFICANT_BIT_INDEX(attack_bb);
            CLEAR_LEAST_SIGNIFICANT_BIT(attack_bb);
            if ((1ULL << to_sqr) & ~(board->occupied_squares_by[BOTH])){
                Moves->moves[Moves->count++] = encode_move(b + pieces_offset, from_square, to_sqr, NO_PIECE, NO_PIECE, QUIET);
            }else{
                Moves->moves[Moves->count++] = encode_move(b + pieces_offset, from_square, to_sqr, NO_PIECE, get_target_piece(board, to_sqr), CAPTURE);
            }
        }
    }

    bb = board->pieces[q + pieces_offset];
    while (bb) {
        from_square = GET_LEAST_SIGNIFICANT_BIT_INDEX(bb);
        CLEAR_LEAST_SIGNIFICANT_BIT(bb);
        attack_bb = get_queen_attacks(board->occupied_squares_by[BOTH], from_square) & empty_or_enemy;
        while(attack_bb){
            to_sqr = GET_LEAST_SIGNIFICANT_BIT_INDEX(attack_bb);
            CLEAR_LEAST_SIGNIFICANT_BIT(attack_bb);
            if ((1ULL << to_sqr) & ~(board->occupied_squares_by[BOTH])){
                Moves->moves[Moves->count++] = encode_move(q + pieces_offset, from_square, to_sqr, NO_PIECE, NO_PIECE, QUIET);
            }else{
                Moves->moves[Moves->count++] = encode_move(q + pieces_offset, from_square, to_sqr, NO_PIECE, get_target_piece(board, to_sqr), CAPTURE);
            }
        }
    }
}

void print_move(u32 move){
    printf("%c     | %s          | %s        | %c               | %c            | %lu         \n",
        pieces_int_to_chr[MOVE_GET_PIECE(move)],
        squares_int_to_chr[MOVE_GET_FROM_SQUARE(move)],
        squares_int_to_chr[MOVE_GET_TO_SQUARE(move)],
        (MOVE_GET_PROMOTION_PIECE(move)) >> 16 == 0 ? '0' : pieces_int_to_chr[MOVE_GET_PROMOTION_PIECE(move)] >> 16,
        pieces_int_to_chr[MOVE_GET_CAPTURE_PIECE(move)],
        (MOVE_GET_FLAG(move))
    );
}

void print_moves(S_Moves* Moves){
    u32 move;
    printf("Moves count: %i\n", Moves->count);
    printf("--------------------------------------------------------------------------------------------------------------------------------------------------------------------------|\n");
    printf("piece | from_square | to_square | promotion_piece | capture_piece | flag |\n");
    printf("--------------------------------------------------------------------------------------------------------------------------------------------------------------------------|\n");
    for (int i = 0; i < Moves->count; i++){
        move = Moves->moves[i];
        printf("%c     | %s          | %s        | %c               | %c            | %lu         \n",
            pieces_int_to_chr[MOVE_GET_PIECE(move)],
            squares_int_to_chr[MOVE_GET_FROM_SQUARE(move)],
            squares_int_to_chr[MOVE_GET_TO_SQUARE(move)],
            (MOVE_GET_PROMOTION_PIECE(move)) >> 16 == 0 ? '0' : pieces_int_to_chr[MOVE_GET_PROMOTION_PIECE(move)] >> 16,
            pieces_int_to_chr[MOVE_GET_CAPTURE_PIECE(move)],
            (MOVE_GET_FLAG(move))
        );
        printf("--------------------------------------------------------------------------------------------------------------------------------------------------------------------------|\n");
    }
}