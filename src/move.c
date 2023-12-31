#include "defs.h"
#include <stdio.h>

static INLINE void handle_promotion(S_Board* board, u8 piece, u8 promotion_piece, u8 to_sqr, u8 color){
    CLEAR_BIT(board->pieces[piece], to_sqr);
    board->hash ^= TT_squares_hash[piece][to_sqr];
    SET_BIT(board->pieces[promotion_piece - 6 * color], to_sqr);
    board->hash ^= TT_squares_hash[promotion_piece - 6 * color][to_sqr];
}

static INLINE void handle_promotion_capture(S_Board* board, u32 move, u8 piece, u8 promotion_piece, u8 to_sqr, u8 color){
    CLEAR_BIT(board->pieces[piece], to_sqr);
    board->hash ^= TT_squares_hash[piece][to_sqr];
    SET_BIT(board->pieces[promotion_piece - 6 * color], to_sqr);
    board->hash ^= TT_squares_hash[promotion_piece - 6 * color][to_sqr];
    CLEAR_BIT(board->pieces[MOVE_GET_CAPTURE_PIECE(move)], to_sqr);
    board->hash ^= TT_squares_hash[MOVE_GET_CAPTURE_PIECE(move)][to_sqr];
    CLEAR_BIT(board->occupied_squares_by[color ^ 1], to_sqr);
}

static INLINE void handle_castling(S_Board* board, u8 rook, u8 rook_from, u8 rook_to, u8 color){
    CLEAR_BIT(board->pieces[rook], rook_from);
    board->hash ^= TT_squares_hash[rook][rook_from];
    CLEAR_BIT(board->occupied_squares_by[color], rook_from);
    SET_BIT(board->pieces[rook], rook_to);
    board->hash ^= TT_squares_hash[rook][rook_to];
    SET_BIT(board->occupied_squares_by[color], rook_to);
}

void make_move(S_Board* board, u32 move){
    const u8 piece = MOVE_GET_PIECE(move), to_sqr = MOVE_GET_TO_SQUARE(move), from_square = MOVE_GET_FROM_SQUARE(move), color = board->sideToMove;

    board->hash ^= TT_enpassant_hash[board->enPassantSquare % 8];
    board->enPassantSquare = A8;

    CLEAR_BIT(board->pieces[piece], from_square);
    CLEAR_BIT(board->occupied_squares_by[color], from_square);
    board->hash ^= TT_squares_hash[piece][from_square];
    SET_BIT(board->pieces[piece], to_sqr);
    SET_BIT(board->occupied_squares_by[color], to_sqr);
    board->hash ^= TT_squares_hash[piece][to_sqr];

    switch (MOVE_GET_FLAG(move)) {
        case DOUBLE_PUSH:
            board->enPassantSquare = to_sqr + 8 - (16 * color);
            board->hash ^= TT_enpassant_hash[board->enPassantSquare % 8];
            break;
        case CAPTURE:
            CLEAR_BIT(board->pieces[MOVE_GET_CAPTURE_PIECE(move)], to_sqr);
            CLEAR_BIT(board->occupied_squares_by[color ^ 1], to_sqr);
            board->hash ^= TT_squares_hash[MOVE_GET_CAPTURE_PIECE(move)][to_sqr];
            break;
        case EP_CAPTURE:
            CLEAR_BIT(board->pieces[MOVE_GET_CAPTURE_PIECE(move)], (to_sqr + 8 - (16 * (color))));
            CLEAR_BIT(board->occupied_squares_by[color ^ 1], (to_sqr + 8 - (16 * (color))));
            board->hash ^= TT_squares_hash[MOVE_GET_CAPTURE_PIECE(move)][to_sqr + 8 - (16 * (color))];
            break;
        case KING_CASTLE: 
        case QUEEN_CASTLE:
            {
                switch (to_sqr) {
                    case G1:
                        handle_castling(board, R, H1, F1, color);
                        break;
                    case G8:
                        handle_castling(board, r, H8, F8, color);
                        break;
                    case C1:
                        handle_castling(board, R, A1, D1, color);
                        break;
                    case C8:
                        handle_castling(board, r, A8, D8, color);
                        break;
                }
            }
            break;
        case PROMOTION_N:
            handle_promotion(board, piece, N, to_sqr, color);
            break;
        case PROMOTION_R:
            handle_promotion(board, piece, R, to_sqr, color);
            break;
        case PROMOTION_B:
            handle_promotion(board, piece, B, to_sqr, color);;
            break;
        case PROMOTION_Q:
            handle_promotion(board, piece, Q, to_sqr, color);
            break;
        case CAPTURE_PROMOTION_N:
            handle_promotion_capture(board, move, piece, N, to_sqr, color);
            break;
        case CAPTURE_PROMOTION_R:
            handle_promotion_capture(board, move, piece, R, to_sqr, color);
            break;
        case CAPTURE_PROMOTION_B:
            handle_promotion_capture(board, move, piece, B, to_sqr, color);
            break;
        case CAPTURE_PROMOTION_Q:
            handle_promotion_capture(board, move, piece, Q, to_sqr, color);
            break;
    }

    board->sideToMove ^= 1;
    board->ply++;
    board->hash ^= board->castlePermission;
    board->castlePermission &= CASTLING_CHANGE_ON_MOVE[from_square];
    board->castlePermission &= CASTLING_CHANGE_ON_MOVE[to_sqr];
    board->hash ^= board->castlePermission;
    board->hash ^= TT_side_to_move_hash;
    board->occupied_squares_by[BOTH] = board->occupied_squares_by[WHITE] | board->occupied_squares_by[BLACK];
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