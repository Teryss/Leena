#include "defs.h"
#include <stdio.h>

/*      STATE ENCODING
    6 bits - fifty moves rule
    6 bits - en passant square
    4 bits - castle permission
    16 bits overall
*/

u16 encode_state(S_Position* Pos){
    return (u16)(
        (Pos->castlePermission & 0b1111) |
        (Pos->fiftyMovesCounter & 0b111111) << 4 |
        Pos->enPassantSquare << 10
    );
}

static inline void promote(S_Position* Pos, u8 piece, u8 promotion_piece, u8 to_sqr, u8 color){
    Pos->Board->pieceSet[to_sqr] = pieces_int_to_chr[promotion_piece] + 32 * color;
    Pos->Board->piecesBB[piece] ^= sqrs[to_sqr];
    Pos->Board->piecesBB[promotion_piece] ^= sqrs[to_sqr];
}

static inline void undo_promote(S_Position* Pos, u8 piece, u8 promotion_piece, u8 toSqr, u8 fromSqr, u8 color){
    Pos->Board->pieceSet[fromSqr] = 'P' + 32 * color;
    Pos->Board->pieceSet[toSqr] = 0;

    Pos->Board->piecesBB[p] ^= sqrs[fromSqr];
    Pos->Board->colorBB[1 - Pos->sideToMove] ^= sqrs[fromSqr];
    Pos->Board->piecesBB[promotion_piece] ^= sqrs[toSqr];
    Pos->Board->colorBB[1 - Pos->sideToMove] ^= sqrs[toSqr];
}

static inline void promote_capture(S_Position* Pos, u16 move, u8 pieceType, u8 capturePieceType, u8 promotion_piece, u8 to_sqr, u8 color){
    Pos->Board->pieceSet[to_sqr] = pieces_int_to_chr[promotion_piece + 6] + 32 * color;
    Pos->Board->piecesBB[pieceType] ^= sqrs[to_sqr];
    Pos->Board->piecesBB[capturePieceType] ^= sqrs[to_sqr];
    Pos->Board->colorBB[1 - color] ^= sqrs[to_sqr];
    Pos->Board->piecesBB[promotion_piece] ^= sqrs[to_sqr];
}

static inline void undo_promotecapture(S_Position* Pos, u16 move, u8 capturePieceType, u8 promotion_piece, u8 to_sqr, u8 fromSqr, u8 color){
    Pos->Board->pieceSet[fromSqr] = 'P' + 32 * color;
    Pos->Board->pieceSet[to_sqr] = 0;

    Pos->Board->piecesBB[p] ^= sqrs[fromSqr];
    Pos->Board->colorBB[1 - Pos->sideToMove] ^= sqrs[fromSqr];

    Pos->Board->piecesBB[promotion_piece] ^= sqrs[to_sqr];
    Pos->Board->colorBB[Pos->sideToMove] ^= sqrs[to_sqr];

    // Pos->Board->piecesBB[promotion_piece] ^= sqrs[to_sqr];
    Pos->Board->colorBB[1 - Pos->sideToMove] ^= sqrs[to_sqr];

    Pos->Board->pieceSet[to_sqr] = pieces_int_to_chr[promotion_piece + 6] + 32 * (1 - color);
    printf("[%d=%c]\n", pieces_int_to_chr[promotion_piece + 6] + 32 * (1 - color), pieces_int_to_chr[promotion_piece + 6] + 32 * (1 - color));
    // Pos->Board->piecesBB[promotion_piece]
}

static inline void castle(S_Position* Pos, u8 rook, u8 rook_from, u8 rook_to, u8 color, u8 pieceTypeFrom){
    Pos->Board->piecesBB[rook] ^= sqrs[rook_from];
    Pos->Board->colorBB[rook] ^= sqrs[rook_from];
    Pos->Board->piecesBB[rook] ^= sqrs[rook_to];
    Pos->Board->colorBB[rook] ^= sqrs[rook_to];

    Pos->Board->pieceSet[rook_from] = 0;    
    // k (int) -> 5, k (ascii) -> 75, r (ascii) -> 82, so... 82 - 5 = 77
    // k -> K (ascii) + 32
    Pos->Board->pieceSet[rook_to] = pieceTypeFrom + 77 + 32 * color; //ascii K + 7 == R
}

static inline void _umove(S_Position* Pos, u8 pieceTypeFrom, u8 fromSqr, u8 toSqr){
    Pos->Board->piecesBB[pieceTypeFrom] ^= sqrs[fromSqr];
    Pos->Board->colorBB[1 - Pos->sideToMove] ^= sqrs[fromSqr];
    Pos->Board->piecesBB[pieceTypeFrom] ^= sqrs[toSqr];
    Pos->Board->colorBB[1 - Pos->sideToMove] ^= sqrs[toSqr];
}


u8 make_move(S_Position* Pos, u16 move){
    const u8 fromSqr = MOVE_FROM_SQUARE(move), toSqr = MOVE_TO_SQUARE(move);
    const u8 pieceTypeFrom = pieceChrToInt(Pos->Board->pieceSet[fromSqr]);
    const u8 pieceTypeTo = pieceChrToInt(Pos->Board->pieceSet[toSqr]);

    // just for a return
    const u8 capturedPiece = Pos->Board->pieceSet[toSqr];
    // printf("|%s|%s| - |%c|%c|\n", squares_int_to_chr[fromSqr], squares_int_to_chr[toSqr], Pos->Board->pieceSet[fromSqr], Pos->Board->pieceSet[toSqr]);

    Pos->Board->pieceSet[toSqr] = Pos->Board->pieceSet[fromSqr];
    Pos->Board->pieceSet[fromSqr] = 0;

    Pos->Board->piecesBB[pieceTypeFrom] ^= sqrs[fromSqr];
    Pos->Board->colorBB[Pos->sideToMove] ^= sqrs[fromSqr];
    Pos->Board->piecesBB[pieceTypeFrom] ^= sqrs[toSqr];
    Pos->Board->colorBB[Pos->sideToMove] ^= sqrs[toSqr];

    // Pos->Board->hash ^= TT_enpassant_hash[Pos->enPassantSquare % 8];
    Pos->enPassantSquare = 0;

    switch (MOVE_GET_FLAG(move)) {
        case QUIET:
            break;
        case DOUBLE_PUSH:
            Pos->enPassantSquare = toSqr + 8 - (16 * Pos->sideToMove);
            // Pos->Board->hash ^= TT_enpassant_hash[Pos->enPassantSquare % 8];
            break;
        case CAPTURE:
            Pos->Board->piecesBB[pieceTypeTo] ^= sqrs[toSqr];
            Pos->Board->colorBB[1 - Pos->sideToMove] ^= sqrs[toSqr];
            // CLEAR_BIT(Pos->Board->piecesBB[pieceTypeTo], toSqr);
            // CLEAR_BIT(Pos->Board->colorBB[1 - Pos->sideToMove], toSqr);
            // Pos->Board->hash ^= TT_squares_hash[pieceTypeTo][toSqr];
            break;
        case EP_CAPTURE:
            Pos->Board->piecesBB[pieceTypeTo] ^= sqrs[toSqr + 8 - (16 * Pos->sideToMove)];
            Pos->Board->colorBB[1 - Pos->sideToMove] ^= sqrs[toSqr + 8 - (16 * Pos->sideToMove)];
            // CLEAR_BIT(Pos->Board->piecesBB[pieceTypeTo], (toSqr + 8 - (16 * (Pos->sideToMove))));
            // CLEAR_BIT(Pos->Board->colorBB[1 - Pos->sideToMove], (toSqr + 8 - (16 * (Pos->sideToMove))));
            // Pos->Board->hash ^= TT_squares_hash[pieceTypeTo][toSqr + 8 - (16 * (Pos->sideToMove))];
            break;
        case KING_CASTLE: 
        case QUEEN_CASTLE:
            {
                switch (toSqr) {
                    case G1:
                        castle(Pos, R, H1, F1, Pos->sideToMove, pieceTypeFrom);
                        break;
                    case G8:
                        castle(Pos, r, H8, F8, Pos->sideToMove, pieceTypeFrom);
                        break;
                    case C1:
                        castle(Pos, R, A1, D1, Pos->sideToMove, pieceTypeFrom);
                        break;
                    case C8:
                        castle(Pos, r, A8, D8, Pos->sideToMove, pieceTypeFrom);
                        break;
                }
            }
            break;
        case PROMOTION_N:
            promote(Pos, pieceTypeFrom, N, toSqr, Pos->sideToMove);
            break;
        case PROMOTION_R:
            promote(Pos, pieceTypeFrom, R, toSqr, Pos->sideToMove);
            break;
        case PROMOTION_B:
            promote(Pos, pieceTypeFrom, B, toSqr, Pos->sideToMove);
            break;
        case PROMOTION_Q:
            promote(Pos, pieceTypeFrom, Q, toSqr, Pos->sideToMove);
            break;
        case CAPTURE_PROMOTION_N:
            promote_capture(Pos, move, pieceTypeFrom, pieceTypeTo, N, toSqr, Pos->sideToMove);
            break;
        case CAPTURE_PROMOTION_R:
            promote_capture(Pos, move, pieceTypeFrom, pieceTypeTo, R, toSqr, Pos->sideToMove);
            break;
        case CAPTURE_PROMOTION_B:
            promote_capture(Pos, move, pieceTypeFrom, pieceTypeTo, B, toSqr, Pos->sideToMove);
            break;
        case CAPTURE_PROMOTION_Q:
            promote_capture(Pos, move, pieceTypeFrom, pieceTypeTo, Q, toSqr, Pos->sideToMove);
            break;   
    }

    Pos->sideToMove = 1 - Pos->sideToMove;
    Pos->ply++;
    // Pos->Board->hash ^= Pos->castlePermission;
    Pos->castlePermission &= CASTLING_CHANGE_ON_MOVE[fromSqr];
    Pos->castlePermission &= CASTLING_CHANGE_ON_MOVE[toSqr];
    // Pos->Board->hash ^= Pos->castlePermission;
    // Pos->Board->hash ^= TT_side_to_move_hash;
    Pos->Board->colorBB[BOTH] = Pos->Board->colorBB[WHITE] | Pos->Board->colorBB[BLACK];

    return capturedPiece;
}

void undo_move(S_Position* Pos, u16 move, u16 lastState, u8 capturePiece){
    const u8 fromSqr = MOVE_FROM_SQUARE(move), toSqr = MOVE_TO_SQUARE(move);
    const u8 pieceTypeFrom = pieceChrToInt(Pos->Board->pieceSet[toSqr]);
    const u8 capturePieceInt = pieceChrToInt(capturePiece);

    Pos->Board->pieceSet[fromSqr] = Pos->Board->pieceSet[toSqr];
    Pos->Board->pieceSet[toSqr] = capturePiece;

    Pos->enPassantSquare = lastState >> 10;

    switch (MOVE_GET_FLAG(move)) {
        case QUIET:
            _umove(Pos, pieceTypeFrom, fromSqr, toSqr);
            break;
        case DOUBLE_PUSH:
            _umove(Pos, pieceTypeFrom, fromSqr, toSqr);
            break;
        case CAPTURE:
            _umove(Pos, pieceTypeFrom, fromSqr, toSqr);
            Pos->Board->piecesBB[capturePieceInt] ^= sqrs[toSqr];
            Pos->Board->colorBB[Pos->sideToMove] ^= sqrs[toSqr];
            break;
        case EP_CAPTURE:
            _umove(Pos, pieceTypeFrom, fromSqr, toSqr);
            Pos->Board->piecesBB[capturePieceInt] ^= sqrs[toSqr + 8 - (16 * (1 - Pos->sideToMove))];
            Pos->Board->colorBB[Pos->sideToMove] ^= sqrs[toSqr + 8 - (16 * (1 - Pos->sideToMove))];
            break;
        case KING_CASTLE: 
        case QUEEN_CASTLE:
            _umove(Pos, pieceTypeFrom, fromSqr, toSqr);
            {
                switch (toSqr) {
                    case G1:
                        castle(Pos, R, F1, H1, 1 - Pos->sideToMove, pieceTypeFrom);
                        break;
                    case G8:
                        castle(Pos, r, F8, H8, 1 - Pos->sideToMove, pieceTypeFrom);
                        break;
                    case C1:
                        castle(Pos, R, D1, A1, 1 - Pos->sideToMove, pieceTypeFrom);
                        break;
                    case C8:
                        castle(Pos, r, D8, A8, 1 - Pos->sideToMove, pieceTypeFrom);
                        break;
                }
            }
            break;
        case PROMOTION_N:
            undo_promote(Pos, pieceTypeFrom, n, toSqr, fromSqr, 1 - Pos->sideToMove);
            break;
        case PROMOTION_R:
            undo_promote(Pos, pieceTypeFrom, r, toSqr, fromSqr, 1 - Pos->sideToMove);
            break;
        case PROMOTION_B:
            undo_promote(Pos, pieceTypeFrom, b, toSqr, fromSqr, 1 - Pos->sideToMove);
            break;
        case PROMOTION_Q:
            undo_promote(Pos, pieceTypeFrom, q, toSqr, fromSqr, 1 - Pos->sideToMove);
            break;
        case CAPTURE_PROMOTION_N:
            undo_promotecapture(Pos, move, capturePieceInt, n, toSqr, fromSqr, 1 - Pos->sideToMove);
            break;
        case CAPTURE_PROMOTION_R:
            undo_promotecapture(Pos, move, capturePieceInt, r, toSqr, fromSqr, 1 - Pos->sideToMove);
            break;
        case CAPTURE_PROMOTION_B:
            undo_promotecapture(Pos, move, capturePieceInt, b, toSqr, fromSqr, 1 - Pos->sideToMove);
            break;
        case CAPTURE_PROMOTION_Q:
            undo_promotecapture(Pos, move, capturePieceInt, q, toSqr, fromSqr, 1 - Pos->sideToMove);
            break;   
    }

    Pos->sideToMove = 1 - Pos->sideToMove;
    Pos->ply--;
    Pos->Board->colorBB[BOTH] = Pos->Board->colorBB[WHITE] | Pos->Board->colorBB[BLACK];

    // Pos->castlePermission &= CASTLING_CHANGE_ON_MOVE[fromSqr];
    // Pos->castlePermission &= CASTLING_CHANGE_ON_MOVE[toSqr];
}

static inline const char* decodeMoveFlag(u8 flag){
    switch (flag){
        case QUIET:
            return "Quiet";
        case DOUBLE_PUSH:
            return "Double Push";
        case KING_CASTLE:
            return "Castling king side";
        case QUEEN_CASTLE:
            return "Castling queen side";
        case PROMOTION_N:
            return "Promotion Knight";
        case PROMOTION_B:
            return "Promotion Bishop";
        case PROMOTION_R:
            return "Promotion Rook";
        case PROMOTION_Q:
            return "Promotion Queen";
        case CAPTURE:
            return "Capture";
        case EP_CAPTURE:
            return "En passant capture";
        case CAPTURE_PROMOTION_N:
            return "Capture promotion Knight";
        case CAPTURE_PROMOTION_B:
            return "Capture promotion Bishop";
        case CAPTURE_PROMOTION_R:
            return "Capture promotion Rook";
        case CAPTURE_PROMOTION_Q:
            return "Capture promotion Queen";
        default:
            return "ERROR";
    }
} 

void print_move(S_Position* Pos, u16 move){
    printf("%c     | %s          | %s        | %c             | %s         \n",
            Pos->Board->pieceSet[MOVE_FROM_SQUARE(move)],
            squares_int_to_chr[MOVE_FROM_SQUARE(move)],
            squares_int_to_chr[MOVE_TO_SQUARE(move)],
            Pos->Board->pieceSet[MOVE_TO_SQUARE(move)] != 0 ? Pos->Board->pieceSet[MOVE_TO_SQUARE(move)] : '.',
            decodeMoveFlag(MOVE_GET_FLAG(move))
        );
}
void print_moves(S_Position* Pos, S_Moves* Moves){
    u16 move;
    printf("Moves count: %i\n", Moves->count);
    printf("----------------------------------------------------------------------|\n");
    printf("piece | from_square | to_square | capture_piece | flag \n");
    printf("----------------------------------------------------------------------|\n");
    for (int i = 0; i < Moves->count; i++){
        move = Moves->moves[i];
        printf("%c     | %s          | %s        | %c             | %s         \n",
            Pos->Board->pieceSet[MOVE_FROM_SQUARE(move)],
            squares_int_to_chr[MOVE_FROM_SQUARE(move)],
            squares_int_to_chr[MOVE_TO_SQUARE(move)],
            Pos->Board->pieceSet[MOVE_TO_SQUARE(move)] != 0 ? Pos->Board->pieceSet[MOVE_TO_SQUARE(move)] : '.',
            decodeMoveFlag(MOVE_GET_FLAG(move))
        );
        printf("----------------------------------------------------------------------|\n");
    }
}