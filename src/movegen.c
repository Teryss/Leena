#include <string.h>
#include "defs.h"

#define RANK_1 0xFFLLU

/* MOVE ENCODING - 32 bits, 28 required

    0000 0000 0000 0000 0000 0000 0000 1111 - piece to move 
    0000 0000 0000 0000 0000 0011 1111 0000 - from square
    0000 0000 0000 0000 1111 1100 0000 0000 - to square
    0000 0000 0000 1111 0000 0000 0000 0000 - promotion piece
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

/*
It's used to checked if a king is in check during move generation
*/
static inline PURE u8 is_square_attacked(const S_Board* const board, const uint square){
    if (get_rook_attacks(board->occupied_squares_by[BOTH], square) & board->pieces[r + 6 * board->sideToMove]) return 1;
    if (get_bishop_attacks(board->occupied_squares_by[BOTH], square) & board->pieces[b + 6 * board->sideToMove]) return 1; 
    if (get_queen_attacks(board->occupied_squares_by[BOTH], square) & board->pieces[q + 6 * board->sideToMove]) return 1;
    if (Masks.knight[square] & board->pieces[n + 6 * board->sideToMove]) return 1;
    if (Masks.pawn_attacks[board->sideToMove][square]  & board->pieces[p + 6 * board->sideToMove]) return 1;
    return 0;
}

void generateOnlyCaptures(const S_Board* const board, S_Moves* Moves){
    const uint side_to_move = board->sideToMove;
    const uint pieces_offset = side_to_move == WHITE ? 6 : 0;
    const uint enemy_color = 1 - side_to_move;
    uint from_square, to_sqr;
    u64 bb, attack_bb = 0;
    Moves->count = 0;

    bb = board->pieces[p + pieces_offset];
    if (board->enPassantSquare){
        u64 ep_bb = Masks.pawn_attacks[enemy_color][board->enPassantSquare] & bb;
        if (ep_bb){
            if (MORE_THAN_ONE(ep_bb)){
                for (uint i = 0; i < 2; i++){
                    from_square = GET_LEAST_SIGNIFICANT_BIT_INDEX(ep_bb);
                    CLEAR_LEAST_SIGNIFICANT_BIT(ep_bb);
                    Moves->moves[Moves->count++] = encode_move(p + pieces_offset, from_square, board->enPassantSquare, NO_PIECE, P - pieces_offset, EP_CAPTURE);
                }
            }else{
                Moves->moves[Moves->count++] = encode_move(p + pieces_offset, GET_LEAST_SIGNIFICANT_BIT_INDEX(ep_bb), board->enPassantSquare, NO_PIECE, P - pieces_offset, EP_CAPTURE);
            }

        }
    }

    while(bb){
        from_square = GET_LEAST_SIGNIFICANT_BIT_INDEX(bb);
        CLEAR_LEAST_SIGNIFICANT_BIT(bb);
        
        attack_bb = Masks.pawn_attacks[side_to_move][from_square] & board->occupied_squares_by[enemy_color];
        while(attack_bb){
            to_sqr = GET_LEAST_SIGNIFICANT_BIT_INDEX(attack_bb);
            CLEAR_LEAST_SIGNIFICANT_BIT(attack_bb);
            if ((GET_BIT((RANK_1 << 8ULL), from_square) && side_to_move == WHITE) || (GET_BIT((RANK_1 << 48ULL), from_square) && side_to_move == BLACK)){
                Moves->moves[Moves->count++] = encode_move(p + pieces_offset, from_square, to_sqr, r + pieces_offset, get_target_piece(board, to_sqr), CAPTURE_PROMOTION_R);
                Moves->moves[Moves->count++] = encode_move(p + pieces_offset, from_square, to_sqr, q + pieces_offset, get_target_piece(board, to_sqr), CAPTURE_PROMOTION_Q);
                Moves->moves[Moves->count++] = encode_move(p + pieces_offset, from_square, to_sqr, n + pieces_offset, get_target_piece(board, to_sqr), CAPTURE_PROMOTION_N);
                Moves->moves[Moves->count++] = encode_move(p + pieces_offset, from_square, to_sqr, b + pieces_offset, get_target_piece(board, to_sqr), CAPTURE_PROMOTION_B);
            }else{
                Moves->moves[Moves->count++] = encode_move(p + pieces_offset, from_square, to_sqr, NO_PIECE, get_target_piece(board, to_sqr), CAPTURE);
            }
        }
    }

    bb = board->pieces[n + pieces_offset];
    while(bb){
        from_square = GET_LEAST_SIGNIFICANT_BIT_INDEX(bb);
        CLEAR_LEAST_SIGNIFICANT_BIT(bb);
        attack_bb = Masks.knight[from_square] & board->occupied_squares_by[enemy_color];

        while(attack_bb){
            to_sqr = GET_LEAST_SIGNIFICANT_BIT_INDEX(attack_bb);
            CLEAR_LEAST_SIGNIFICANT_BIT(attack_bb);
            Moves->moves[Moves->count++] = encode_move(n + pieces_offset, from_square, to_sqr, NO_PIECE, get_target_piece(board, to_sqr), CAPTURE);
        }
    }

    bb = board->pieces[r + pieces_offset];
    while (bb) {
        from_square = GET_LEAST_SIGNIFICANT_BIT_INDEX(bb);
        CLEAR_LEAST_SIGNIFICANT_BIT(bb);
        attack_bb = get_rook_attacks(board->occupied_squares_by[BOTH], from_square) & board->occupied_squares_by[enemy_color];

        while(attack_bb){
            to_sqr = GET_LEAST_SIGNIFICANT_BIT_INDEX(attack_bb);
            CLEAR_LEAST_SIGNIFICANT_BIT(attack_bb);
            Moves->moves[Moves->count++] = encode_move(r + pieces_offset, from_square, to_sqr, NO_PIECE, get_target_piece(board, to_sqr), CAPTURE);
        }
    }

    bb = board->pieces[b + pieces_offset];
    while (bb) {
        from_square = GET_LEAST_SIGNIFICANT_BIT_INDEX(bb);
        CLEAR_LEAST_SIGNIFICANT_BIT(bb);
        attack_bb = get_bishop_attacks(board->occupied_squares_by[BOTH], from_square) & board->occupied_squares_by[enemy_color];

        while(attack_bb){
            to_sqr = GET_LEAST_SIGNIFICANT_BIT_INDEX(attack_bb);
            CLEAR_LEAST_SIGNIFICANT_BIT(attack_bb);
            Moves->moves[Moves->count++] = encode_move(b + pieces_offset, from_square, to_sqr, NO_PIECE, get_target_piece(board, to_sqr), CAPTURE);
        }
    }

    bb = board->pieces[q + pieces_offset];
    while (bb) {
        from_square = GET_LEAST_SIGNIFICANT_BIT_INDEX(bb);
        CLEAR_LEAST_SIGNIFICANT_BIT(bb);
        attack_bb = get_queen_attacks(board->occupied_squares_by[BOTH], from_square) & board->occupied_squares_by[enemy_color];
        while(attack_bb){
            to_sqr = GET_LEAST_SIGNIFICANT_BIT_INDEX(attack_bb);
            CLEAR_LEAST_SIGNIFICANT_BIT(attack_bb);
            Moves->moves[Moves->count++] = encode_move(q + pieces_offset, from_square, to_sqr, NO_PIECE, get_target_piece(board, to_sqr), CAPTURE);
        }
    }

    bb = board->pieces[k + pieces_offset];

    // during search we never allow for a king capture, so no need for while loop

    from_square = GET_LEAST_SIGNIFICANT_BIT_INDEX(bb);
    CLEAR_LEAST_SIGNIFICANT_BIT(bb);
    attack_bb = Masks.king[from_square] & board->occupied_squares_by[enemy_color];

    while(attack_bb){
        to_sqr = GET_LEAST_SIGNIFICANT_BIT_INDEX(attack_bb);
        CLEAR_LEAST_SIGNIFICANT_BIT(attack_bb);
        Moves->moves[Moves->count++] = encode_move(k + pieces_offset, from_square, to_sqr, NO_PIECE, get_target_piece(board, to_sqr), CAPTURE);
    }
}

void generateMoves(const S_Board* const board, S_Moves* Moves){
    const u64 empty_or_enemy = ~board->occupied_squares_by[board->sideToMove];
    u64 bb, attack_bb = 0;
    const u8 side_to_move = board->sideToMove;
    const u8 pieces_offset = side_to_move == WHITE ? 6 : 0;
    const u8 enemy_color = 1 - side_to_move;

    uint from_square, to_sqr;
    Moves->count = 0;

    bb = board->pieces[p + pieces_offset];
    if (board->enPassantSquare){
        u64 ep_bb = Masks.pawn_attacks[enemy_color][board->enPassantSquare] & bb;
        if (ep_bb){
            if (MORE_THAN_ONE(ep_bb)){
                for (uint i = 0; i < 2; i++){
                    from_square = GET_LEAST_SIGNIFICANT_BIT_INDEX(ep_bb);
                    CLEAR_LEAST_SIGNIFICANT_BIT(ep_bb);
                    Moves->moves[Moves->count++] = encode_move(p + pieces_offset, from_square, board->enPassantSquare, NO_PIECE, P - pieces_offset, EP_CAPTURE);
                }
            }else{
                Moves->moves[Moves->count++] = encode_move(p + pieces_offset, GET_LEAST_SIGNIFICANT_BIT_INDEX(ep_bb), board->enPassantSquare, NO_PIECE, P - pieces_offset, EP_CAPTURE);
            }

        }
    }
    while(bb){
        from_square = GET_LEAST_SIGNIFICANT_BIT_INDEX(bb);
        CLEAR_LEAST_SIGNIFICANT_BIT(bb);

        uint temp_square = from_square + 8 - 16 * enemy_color;
        if(!(board->occupied_squares_by[BOTH] & (1ULL << temp_square))){
            if ((GET_BIT((RANK_1 << 8ULL), from_square) && side_to_move == WHITE) || (GET_BIT((RANK_1 << 48ULL), from_square) && side_to_move == BLACK)){
                Moves->moves[Moves->count++] = encode_move(p + pieces_offset, from_square, temp_square, r + pieces_offset, NO_PIECE, PROMOTION_R);
                Moves->moves[Moves->count++] = encode_move(p + pieces_offset, from_square, temp_square, q + pieces_offset, NO_PIECE, PROMOTION_Q);
                Moves->moves[Moves->count++] = encode_move(p + pieces_offset, from_square, temp_square, n + pieces_offset, NO_PIECE, PROMOTION_N);
                Moves->moves[Moves->count++] = encode_move(p + pieces_offset, from_square, temp_square, b + pieces_offset, NO_PIECE, PROMOTION_B);
            }else{
                Moves->moves[Moves->count++] = encode_move(p + pieces_offset, from_square, temp_square, NO_PIECE, NO_PIECE, QUIET);

                temp_square += 8 - 16 * enemy_color;
                if((1ULL << from_square) & RANK_1 << (u64)(8 + (40 * enemy_color))){
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
            if ((GET_BIT((RANK_1 << 8ULL), from_square) && side_to_move == WHITE) || (GET_BIT((RANK_1 << 48ULL), from_square) && side_to_move == BLACK)){
                Moves->moves[Moves->count++] = encode_move(p + pieces_offset, from_square, to_sqr, r + pieces_offset, get_target_piece(board, to_sqr), CAPTURE_PROMOTION_R);
                Moves->moves[Moves->count++] = encode_move(p + pieces_offset, from_square, to_sqr, q + pieces_offset, get_target_piece(board, to_sqr), CAPTURE_PROMOTION_Q);
                Moves->moves[Moves->count++] = encode_move(p + pieces_offset, from_square, to_sqr, n + pieces_offset, get_target_piece(board, to_sqr), CAPTURE_PROMOTION_N);
                Moves->moves[Moves->count++] = encode_move(p + pieces_offset, from_square, to_sqr, b + pieces_offset, get_target_piece(board, to_sqr), CAPTURE_PROMOTION_B);
            }else{
                Moves->moves[Moves->count++] = encode_move(p + pieces_offset, from_square, to_sqr, NO_PIECE, get_target_piece(board, to_sqr), CAPTURE);
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

    bb = board->pieces[k + pieces_offset];

    // during search we never allow for a king capture, so no need for while loop

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