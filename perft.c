#include "defs.h"
#include <stdio.h>
#include <string.h>

u64 perft(S_Board* Board, uint depth);

u64 run_perft(S_Board* Board, uint depth){
    u16 state = encode_state(Board);
    S_Moves Moves;
    u64 nodes = 0ULL, current_nodes;
    generateMoves(Board, &Moves);
    printf("Nodes searched per move:\n");

    for (int i = 0; i < Moves.count; i++){
        current_nodes = 0ULL;

        make_move(Board, Moves.moves[i]);
        if (!(is_king_attacked(Board))){
            current_nodes += perft(Board, depth - 1);
        }

        if (current_nodes != 0)
            printf("%s%s%c: %lu\n", 
                    squares_int_to_chr[MOVE_GET_FROM_SQUARE(Moves.moves[i])],
                squares_int_to_chr[MOVE_GET_TO_SQUARE(Moves.moves[i])],
                (MOVE_GET_PROMOTION_PIECE(Moves.moves[i]) != NO_PIECE ? pieces_int_to_chr[MOVE_GET_PROMOTION_PIECE(Moves.moves[i])] : ' '),
                current_nodes);
        undo_move(Board, state);
        nodes += current_nodes;
    }

    return nodes;
}

u64 perft(S_Board* Board, uint depth){
    if (depth == 0 || is_king_attacked(Board)) { return 1ULL; }

    u64 nodes = 0ULL;
    u16 state = encode_state(Board);
    S_Moves Moves;
    generateMoves(Board, &Moves);
    for (int i = 0; i < Moves.count; i++){
        make_move(Board, Moves.moves[i]);
        if ((is_king_attacked(Board)) == 0){
            nodes += perft(Board, depth - 1);   
        }
        undo_move(Board, state);
    }

    return nodes;
}

void make_move(S_Board* board, u32 move){
    const u8 piece = MOVE_GET_PIECE(move), to_sqr = MOVE_GET_TO_SQUARE(move), from_square = MOVE_GET_FROM_SQUARE(move), color = board->sideToMove;
    board->enPassantSquare = NO_SQR;

    CLEAR_BIT(board->pieces[piece], from_square);
    CLEAR_BIT(board->occupied_squares_by[color], from_square);
    SET_BIT(board->pieces[piece], to_sqr);
    SET_BIT(board->occupied_squares_by[color], to_sqr);

    switch (MOVE_GET_FLAG(move)) {
        case DOUBLE_PUSH:
            board->enPassantSquare = to_sqr + 8 - (16 * color);
            break;
        case CAPTURE:
            CLEAR_BIT(board->pieces[MOVE_GET_CAPTURE_PIECE(move)], to_sqr);
            CLEAR_BIT(board->occupied_squares_by[color ^ 1], to_sqr);
            break;
        case EP_CAPTURE:
            CLEAR_BIT(board->pieces[MOVE_GET_CAPTURE_PIECE(move)], (to_sqr + 8 - (16 * (color))));
            CLEAR_BIT(board->occupied_squares_by[color ^ 1], (to_sqr + 8 - (16 * (color))));
            break;
        case KING_CASTLE: case QUEEN_CASTLE:
            switch (to_sqr) {
                case G1:
                    CLEAR_BIT(board->pieces[R], H1);
                    CLEAR_BIT(board->occupied_squares_by[color], H1);
                    SET_BIT(board->pieces[R], F1);
                    SET_BIT(board->occupied_squares_by[color], F1);
                    break;
                case G8:
                    CLEAR_BIT(board->pieces[r], H8);
                    CLEAR_BIT(board->occupied_squares_by[color], H8);
                    SET_BIT(board->pieces[r], F8);
                    SET_BIT(board->occupied_squares_by[color], F8);
                    break;
                case C1:
                    CLEAR_BIT(board->pieces[R], A1);
                    CLEAR_BIT(board->occupied_squares_by[color], A1);
                    SET_BIT(board->pieces[R], D1);
                    SET_BIT(board->occupied_squares_by[color], D1);
                    break;
                case C8:
                    CLEAR_BIT(board->pieces[r], A8);
                    CLEAR_BIT(board->occupied_squares_by[color], A8);
                    SET_BIT(board->pieces[r], D8);
                    SET_BIT(board->occupied_squares_by[color], D8);
                    break;
            }
            break;
        case PROMOTION_N:
            CLEAR_BIT(board->pieces[piece], to_sqr);
            SET_BIT(board->pieces[n + color], to_sqr);
            break;
        case PROMOTION_R:
            CLEAR_BIT(board->pieces[piece], to_sqr);
            SET_BIT(board->pieces[r + color], to_sqr);
            break;
        case PROMOTION_B:
            CLEAR_BIT(board->pieces[piece], to_sqr);
            SET_BIT(board->pieces[b + color], to_sqr);
            break;
        case PROMOTION_Q:
            CLEAR_BIT(board->pieces[piece], to_sqr);
            SET_BIT(board->pieces[q + color], to_sqr);
            break;
        case CAPTURE_PROMOTION_N:
            CLEAR_BIT(board->pieces[piece], to_sqr);
            SET_BIT(board->pieces[n + color], to_sqr);
            CLEAR_BIT(board->pieces[MOVE_GET_CAPTURE_PIECE(move)], to_sqr);
            CLEAR_BIT(board->occupied_squares_by[color ^ 1], to_sqr);
            break;
        case CAPTURE_PROMOTION_R:
            CLEAR_BIT(board->pieces[piece], to_sqr);
            SET_BIT(board->pieces[r + color], to_sqr);
            CLEAR_BIT(board->pieces[MOVE_GET_CAPTURE_PIECE(move)], to_sqr);
            CLEAR_BIT(board->occupied_squares_by[color ^ 1], to_sqr);
            break;
        case CAPTURE_PROMOTION_B:
            CLEAR_BIT(board->pieces[piece], to_sqr);
            SET_BIT(board->pieces[b + color], to_sqr);
            CLEAR_BIT(board->pieces[MOVE_GET_CAPTURE_PIECE(move)], to_sqr);
            CLEAR_BIT(board->occupied_squares_by[color ^ 1], to_sqr);
            break;
        case CAPTURE_PROMOTION_Q:
            CLEAR_BIT(board->pieces[piece], to_sqr);
            SET_BIT(board->pieces[q + color], to_sqr);
            CLEAR_BIT(board->pieces[MOVE_GET_CAPTURE_PIECE(move)], to_sqr);
            CLEAR_BIT(board->occupied_squares_by[color ^ 1], to_sqr);
            break;
    }

    board->sideToMove ^= 1;
    board->move_history[board->ply] = move;
    board->ply++;
    board->castlePermission &= CASTLING_CHANGE_ON_MOVE[from_square];
    board->castlePermission &= CASTLING_CHANGE_ON_MOVE[to_sqr];
    board->occupied_squares_by[BOTH] = board->occupied_squares_by[WHITE] | board->occupied_squares_by[BLACK];

}

void undo_move(S_Board* board, u16 previous_state){
    board->ply--;
    const u32 move = board->move_history[board->ply];
    const u8 piece = MOVE_GET_PIECE(move), to_sqr = MOVE_GET_TO_SQUARE(move), from_square = MOVE_GET_FROM_SQUARE(move), color = board->sideToMove ^ 1;

    SET_BIT(board->pieces[piece], from_square);
    SET_BIT(board->occupied_squares_by[color], from_square);
    CLEAR_BIT(board->pieces[piece], to_sqr);
    CLEAR_BIT(board->occupied_squares_by[color], to_sqr);

    switch (MOVE_GET_FLAG(move)) {
        case DOUBLE_PUSH:
            board->enPassantSquare = to_sqr + 8 - (16 * color);
            break;
        case CAPTURE:
            SET_BIT(board->pieces[MOVE_GET_CAPTURE_PIECE(move)], to_sqr);
            SET_BIT(board->occupied_squares_by[color ^ 1], to_sqr);
            break;
        case EP_CAPTURE:
            SET_BIT(board->pieces[MOVE_GET_CAPTURE_PIECE(move)], (to_sqr + 8 - (16 * (board->sideToMove ^ 1))));
            SET_BIT(board->occupied_squares_by[color ^ 1], (to_sqr + 8 - (16 * (board->sideToMove ^ 1))));
            break;
        case KING_CASTLE: case QUEEN_CASTLE:
            switch (to_sqr) {
                case G1:
                    SET_BIT(board->pieces[R], H1);
                    SET_BIT(board->occupied_squares_by[color], H1);
                    CLEAR_BIT(board->pieces[R], F1);
                    CLEAR_BIT(board->occupied_squares_by[color], F1);
                    break;
                case G8:
                    SET_BIT(board->pieces[r], H8);
                    SET_BIT(board->occupied_squares_by[color], H8);
                    CLEAR_BIT(board->pieces[r], F8);
                    CLEAR_BIT(board->occupied_squares_by[color], F8);
                case C1:
                    SET_BIT(board->pieces[R], A1);
                    SET_BIT(board->occupied_squares_by[color], A1);
                    CLEAR_BIT(board->pieces[R], D1);
                    CLEAR_BIT(board->occupied_squares_by[color], D1);
                    break;
                case C8:
                    SET_BIT(board->pieces[r], A8);
                    SET_BIT(board->occupied_squares_by[color], A8);
                    CLEAR_BIT(board->pieces[r], D8);
                    CLEAR_BIT(board->occupied_squares_by[color], D8);
                    break;
            }
            break;
        case PROMOTION_N:
            CLEAR_BIT(board->pieces[n + color], to_sqr);
            break;
        case PROMOTION_R:
            CLEAR_BIT(board->pieces[r + color], to_sqr);
            break;
        case PROMOTION_B:
            CLEAR_BIT(board->pieces[b + color], to_sqr);
            break;
        case PROMOTION_Q:
            CLEAR_BIT(board->pieces[q + color], to_sqr);
            break;
        case CAPTURE_PROMOTION_N:
            CLEAR_BIT(board->pieces[n + color], to_sqr);
            SET_BIT(board->pieces[MOVE_GET_CAPTURE_PIECE(move)], to_sqr);
            SET_BIT(board->occupied_squares_by[color ^ 1], to_sqr);
            break;
        case CAPTURE_PROMOTION_R:
            CLEAR_BIT(board->pieces[r + color], to_sqr);
            SET_BIT(board->pieces[MOVE_GET_CAPTURE_PIECE(move)], to_sqr);
            SET_BIT(board->occupied_squares_by[color ^ 1], to_sqr);
            break;
        case CAPTURE_PROMOTION_B:
            CLEAR_BIT(board->pieces[b + color], to_sqr);
            SET_BIT(board->pieces[MOVE_GET_CAPTURE_PIECE(move)], to_sqr);
            SET_BIT(board->occupied_squares_by[color ^ 1], to_sqr);
            break;
        case CAPTURE_PROMOTION_Q:
            CLEAR_BIT(board->pieces[q + color], to_sqr);
            SET_BIT(board->pieces[MOVE_GET_CAPTURE_PIECE(move)], to_sqr);
            SET_BIT(board->occupied_squares_by[color ^ 1], to_sqr);
            break;
    }
    board->sideToMove ^= 1;
    board->enPassantSquare = STATE_GET_EN_PASSANT_SQR(previous_state);
    board->castlePermission = STATE_GET_CASTLE_PERM(previous_state);
    board->occupied_squares_by[BOTH] = board->occupied_squares_by[WHITE] | board->occupied_squares_by[BLACK];
}