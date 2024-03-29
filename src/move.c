#include "defs.h"
#include <stdio.h>

/*      STATE ENCODING
    6 bits - fifty moves rule
    6 bits - en passant square
    4 bits - castle permission
*/

static INLINE void castle(S_Position* Pos, u8 rook_from, u8 rook_to, u8 color){
    // move the rook
    Pos->Board->piecesBB[r] ^= sqrs(rook_from) | sqrs(rook_to);
    Pos->Board->colorBB[color] ^= sqrs(rook_from) | sqrs(rook_to);

    // update piece set
    Pos->Board->pieceSet[rook_from] = NO_PIECE;
    Pos->Board->pieceSet[rook_to] = r;
    
    Pos->Board->hash ^= TT_squares_hash[color][r][rook_from] ^ TT_squares_hash[color][r][rook_to]; 
}

static INLINE void promote(S_Position* Pos, u8 promotion_piece, u8 to_sqr){
    /*  At the start of make_move function we move the piece without thinking about promotions/
        Promotions are rather rare, so we just clear the piece from the piece bb    */
    Pos->Board->piecesBB[p] ^= sqrs(to_sqr);
    // Spawn promoted piece
    Pos->Board->pieceSet[to_sqr] = promotion_piece;
    Pos->Board->piecesBB[promotion_piece] ^= sqrs(to_sqr);

    Pos->Board->hash ^= TT_squares_hash[Pos->sideToMove][p][to_sqr] ^ TT_squares_hash[Pos->sideToMove][promotion_piece][to_sqr]; 
}

static INLINE void promote_capture(S_Position* Pos, u8 promotion_piece, u8 to_sqr, u8 capturePieceType, u8 enemyColor){
    // clear the enemy
    Pos->Board->piecesBB[capturePieceType] ^= sqrs(to_sqr);
    // Pos->Board->colorBB[enemyColor] ^= sqrs(to_sqr);
    Pos->Board->colorBB[enemyColor] ^= sqrs(to_sqr);

    Pos->Board->hash ^= TT_squares_hash[Pos->sideToMove][capturePieceType][to_sqr];
    // promote
    promote(Pos, promotion_piece, to_sqr);
}

// static INLINE int get_piece(S_Board* Board, int square){
//     for (int i = 0; i < 6; i++){
//         if (Board->piecesBB[i] & Board->colorBB[BOTH] & (1ULL << square)){
//             return i;
//         }
//     }
//     return NO_PIECE;
// }

void make_move(S_Position* Pos, u16 move){
    const u8 fromSqr = MOVE_FROM_SQUARE(move), toSqr = MOVE_TO_SQUARE(move);
    const u8 pieceTypeTo = Pos->Board->pieceSet[toSqr];

    Pos->Board->hash ^= TT_enpassant_hash[Pos->enPassantSquare % 8];

    ASSERT(Pos->Board->pieceSet[fromSqr] < 6);

    if (fromSqr == toSqr)
        __builtin_unreachable();

    Pos->enPassantSquare = 0;
    if (Pos->sideToMove == WHITE){
        Pos->Board->hash ^= TT_squares_hash[WHITE][Pos->Board->pieceSet[fromSqr]][fromSqr]
            ^ TT_squares_hash[WHITE][Pos->Board->pieceSet[fromSqr]][toSqr];
        // Move the moving piece, update piece and color bb
        Pos->Board->colorBB[WHITE] ^= sqrs(fromSqr) | sqrs(toSqr);
        Pos->Board->piecesBB[Pos->Board->pieceSet[fromSqr]] ^= sqrs(fromSqr) | sqrs(toSqr);
        // Update piece set
        Pos->Board->pieceSet[toSqr] = Pos->Board->pieceSet[fromSqr];
        Pos->Board->pieceSet[fromSqr] = NO_PIECE;


        switch (MOVE_GET_FLAG(move)) {
            case QUIET:
                break;
            case DOUBLE_PUSH:
                Pos->enPassantSquare = toSqr + 8;

                Pos->Board->hash ^= TT_enpassant_hash[toSqr % 8];
                break;
            case CAPTURE:
                // clear enemy piece
                Pos->Board->piecesBB[pieceTypeTo] ^= sqrs(toSqr);
                Pos->Board->colorBB[BLACK] ^= sqrs(toSqr);

                Pos->Board->hash ^= TT_squares_hash[WHITE][pieceTypeTo][toSqr];
                break;
            case EP_CAPTURE:
                // clear enemy pawn
                Pos->Board->pieceSet[toSqr + 8] = NO_PIECE;
                Pos->Board->piecesBB[p] ^= sqrs(toSqr + 8);
                Pos->Board->colorBB[BLACK] ^= sqrs(toSqr + 8);

                Pos->Board->hash ^= TT_squares_hash[WHITE][pieceTypeTo][toSqr + 8];
                break;
            case KING_CASTLE: 
            case QUEEN_CASTLE:
                switch (toSqr) {
                    case G1:
                        castle(Pos, H1, F1, WHITE);
                        break;
                    case G8:
                        castle(Pos, H8, F8, WHITE);
                        break;
                    case C1:
                        castle(Pos, A1, D1, WHITE);
                        break;
                    case C8:
                        castle(Pos, A8, D8, WHITE);
                        break;
                }
                break;
            case PROMOTION_N:
                promote(Pos, n, toSqr);
                break;
            case PROMOTION_R:
                promote(Pos, r, toSqr);
                break;
            case PROMOTION_B:
                promote(Pos, b, toSqr);
                break;
            case PROMOTION_Q:
                promote(Pos, q, toSqr);
                break;
            case CAPTURE_PROMOTION_N:
                promote_capture(Pos, n, toSqr, pieceTypeTo, BLACK);
                break;
            case CAPTURE_PROMOTION_R:
                promote_capture(Pos, r, toSqr, pieceTypeTo, BLACK);
                break;
            case CAPTURE_PROMOTION_B:
                promote_capture(Pos, b, toSqr, pieceTypeTo, BLACK);
                break;
            case CAPTURE_PROMOTION_Q:
                promote_capture(Pos, q, toSqr, pieceTypeTo, BLACK);
                break;
            default:
                __builtin_unreachable();
        }   
        Pos->sideToMove = BLACK;
    }else{
        Pos->Board->hash ^= TT_squares_hash[BLACK][Pos->Board->pieceSet[fromSqr]][fromSqr]
            ^ TT_squares_hash[BLACK][Pos->Board->pieceSet[fromSqr]][toSqr];
        // Move the moving piece, update piece and color bb
        Pos->Board->colorBB[BLACK] ^= sqrs(fromSqr) | sqrs(toSqr);
        Pos->Board->piecesBB[Pos->Board->pieceSet[fromSqr]] ^= sqrs(fromSqr) | sqrs(toSqr);
        // Update piece set
        Pos->Board->pieceSet[toSqr] = Pos->Board->pieceSet[fromSqr];
        Pos->Board->pieceSet[fromSqr] = NO_PIECE;

        switch (MOVE_GET_FLAG(move)) {
            case QUIET:
                break;
            case DOUBLE_PUSH:
                Pos->enPassantSquare = toSqr - 8;

                Pos->Board->hash = TT_enpassant_hash[toSqr % 8];
                break;
            case CAPTURE:
                // clear enemy piece
                Pos->Board->piecesBB[pieceTypeTo] ^= sqrs(toSqr);
                Pos->Board->colorBB[WHITE] ^= sqrs(toSqr);

                Pos->Board->hash ^= TT_squares_hash[WHITE][pieceTypeTo][toSqr];
                break;
            case EP_CAPTURE:
                // clear enemy pawn
                Pos->Board->pieceSet[toSqr - 8] = NO_PIECE;
                Pos->Board->piecesBB[p] ^= sqrs(toSqr - 8);
                Pos->Board->colorBB[WHITE] ^= sqrs(toSqr - 8);

                Pos->Board->hash ^= TT_squares_hash[WHITE][pieceTypeTo][toSqr - 8];
                break;
            case KING_CASTLE: 
            case QUEEN_CASTLE:
                switch (toSqr) {
                    case G1:
                        castle(Pos, H1, F1, BLACK);
                        break;
                    case G8:
                        castle(Pos, H8, F8, BLACK);
                        break;
                    case C1:
                        castle(Pos, A1, D1, BLACK);
                        break;
                    case C8:
                        castle(Pos, A8, D8, BLACK);
                        break;
                }
                break;
            case PROMOTION_N:
                promote(Pos, n, toSqr);
                break;
            case PROMOTION_R:
                promote(Pos, r, toSqr);
                break;
            case PROMOTION_B:
                promote(Pos, b, toSqr);
                break;
            case PROMOTION_Q:
                promote(Pos, q, toSqr);
                break;
            case CAPTURE_PROMOTION_N:
                promote_capture(Pos, n, toSqr, pieceTypeTo, WHITE);
                break;
            case CAPTURE_PROMOTION_R:
                promote_capture(Pos, r, toSqr, pieceTypeTo, WHITE);
                break;
            case CAPTURE_PROMOTION_B:
                promote_capture(Pos, b, toSqr, pieceTypeTo, WHITE);
                break;
            case CAPTURE_PROMOTION_Q:
                promote_capture(Pos, q, toSqr, pieceTypeTo, WHITE);
                break;
            default:
                __builtin_unreachable();
        }   
        Pos->sideToMove = WHITE;
    }

    Pos->Board->hash ^= TT_side_to_move_hash;
    Pos->Board->hash ^= TT_castling_rights_hash[Pos->castlePermission];

    Pos->ply++;
    Pos->castlePermission &= CASTLING_CHANGE_ON_MOVE[fromSqr];
    Pos->castlePermission &= CASTLING_CHANGE_ON_MOVE[toSqr];
    Pos->Board->hash ^= TT_castling_rights_hash[Pos->castlePermission];

    Pos->Board->colorBB[BOTH] = Pos->Board->colorBB[WHITE] | Pos->Board->colorBB[BLACK];

    // return pieceTypeTo;
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

void print_move(S_Board* Board, u16 move){
    printf("%c     | %s          | %s        | %c             | %s         \n",
        pieces_int_to_chr[Board->pieceSet[MOVE_FROM_SQUARE(move)] + (Board->colorBB[WHITE] & sqrs(MOVE_FROM_SQUARE(move)) ? 6 : 0)],
        squares_int_to_chr[MOVE_FROM_SQUARE(move)],
        squares_int_to_chr[MOVE_TO_SQUARE(move)],
        pieces_int_to_chr[Board->pieceSet[MOVE_TO_SQUARE(move)] + (Board->colorBB[WHITE] & sqrs(MOVE_TO_SQUARE(move)) ? 6 : 0)],
        decodeMoveFlag(MOVE_GET_FLAG(move))
    );
}
void print_moves(S_Board* Board, S_Moves* Moves){
    u16 move;
    printf("Moves count: %i\n", Moves->count);
    printf("-----------------------------------------------------------------------------|\n");
    printf("piece | from_square | to_square | capture_piece | flag \n");
    printf("-----------------------------------------------------------------------------|\n");
    for (int i = 0; i < Moves->count; i++){
        move = Moves->moves[i];
        printf("%c     | %s          | %s        | %c             | %s         \n",
            pieces_int_to_chr[Board->pieceSet[MOVE_FROM_SQUARE(move)] + (Board->colorBB[WHITE] & sqrs(MOVE_FROM_SQUARE(move)) ? 6 : 0)],
            squares_int_to_chr[MOVE_FROM_SQUARE(move)],
            squares_int_to_chr[MOVE_TO_SQUARE(move)],
            pieces_int_to_chr[Board->pieceSet[MOVE_TO_SQUARE(move)] + (Board->colorBB[WHITE] & sqrs(MOVE_TO_SQUARE(move)) ? 6 : 0)],
            decodeMoveFlag(MOVE_GET_FLAG(move))
        );
        printf("-----------------------------------------------------------------------------|\n");
    }
}

/*  It's not used, unmaking move is a lot slower than copying and pasting the board.
    I don't understand why make/unmake functions are so slow :(
*/
// void undo_move(S_Position* Pos, u16 move, u16 lastState, u8 capturePiece){
//     const u8 fromSqr = MOVE_FROM_SQUARE(move), toSqr = MOVE_TO_SQUARE(move);
//     const u8 pieceTypeFrom = Pos->Board->pieceSet[toSqr];
//     const u8 pieceTypeTo = capturePiece;
//     const u8 us = 1 - Pos->sideToMove; 
//     const u8 enemy = Pos->sideToMove;
    
//     // Pos->enPassantSquare = 0;
    
//     switch (MOVE_GET_FLAG(move)) {
//         case QUIET:
//             unmove_piece(Pos, fromSqr, toSqr, pieceTypeFrom, us, capturePiece);
//             break;
//         case DOUBLE_PUSH:
//             unmove_piece(Pos, fromSqr, toSqr, pieceTypeFrom, us, capturePiece);
//             Pos->enPassantSquare = toSqr + 8 - (16 * enemy);
//             break;
//         case CAPTURE:
//             // clear enemy piece
//             unmove_piece(Pos, fromSqr, toSqr, pieceTypeFrom, us, capturePiece);
//             Pos->Board->piecesBB[pieceTypeTo] ^= sqrs(toSqr);
//             Pos->Board->colorBB[enemy] ^= sqrs(toSqr);
//             break;
//         case EP_CAPTURE:
//             unmove_piece(Pos, fromSqr, toSqr, pieceTypeFrom, us, capturePiece);
//             // clear enemy pawn
//             Pos->Board->pieceSet[toSqr + 8 - (16 * us)] = p;
//             Pos->Board->piecesBB[pieceTypeFrom] ^= sqrs(toSqr + 8 - (16 * us));
//             Pos->Board->colorBB[enemy] ^= sqrs(toSqr + 8 - (16 * us));
//             break;
//         case KING_CASTLE: 
//         case QUEEN_CASTLE:
//             unmove_piece(Pos, fromSqr, toSqr, pieceTypeFrom, us, capturePiece);
//             switch (toSqr) {
//                 case G1:
//                     castle(Pos, F1, H1, us);
//                     break;
//                 case G8:
//                     castle(Pos, F8, H8, us);
//                     break;
//                 case C1:
//                     castle(Pos, D1, A1, us);
//                     break;
//                 case C8:
//                     castle(Pos, D8, A8, us);
//                     break;
//             }
//             break;
//         case PROMOTION_N:
//             undo_promote(Pos, pieceTypeFrom, n, fromSqr, toSqr, us);
//             break;
//         case PROMOTION_R:
//             undo_promote(Pos, pieceTypeFrom, r, fromSqr, toSqr, us);
//             break;
//         case PROMOTION_B:
//             undo_promote(Pos, pieceTypeFrom, b, fromSqr, toSqr, us);
//             break;
//         case PROMOTION_Q:
//             undo_promote(Pos, pieceTypeFrom, q, fromSqr, toSqr, us);
//             break;
//         case CAPTURE_PROMOTION_N:
//             undo_promote_capture(Pos, pieceTypeFrom, n, fromSqr, toSqr, capturePiece, enemy);
//             break;
//         case CAPTURE_PROMOTION_R:
//             undo_promote_capture(Pos, pieceTypeFrom, r, fromSqr, toSqr, capturePiece, enemy);
//             break;
//         case CAPTURE_PROMOTION_B:
//             undo_promote_capture(Pos, pieceTypeFrom, b, fromSqr, toSqr, capturePiece, enemy);
//             break;
//         case CAPTURE_PROMOTION_Q:
//             undo_promote_capture(Pos, pieceTypeFrom, q, fromSqr, toSqr, capturePiece, enemy);
//             break;
//         default:
//             __builtin_unreachable();
//     }

//     if (Pos->sideToMove != enemy)
//         __builtin_unreachable();

//     Pos->sideToMove = us;
//     Pos->ply--;
//     Pos->castlePermission = GET_CASTLE_PERM(lastState);
//     Pos->enPassantSquare = GET_EN_PASSANT_SQR(lastState);
//     Pos->Board->colorBB[BOTH] = Pos->Board->colorBB[WHITE] | Pos->Board->colorBB[BLACK];
// }


// static inline void unmove_piece(S_Position* Pos, u8 fromSqr, u8 toSqr, u8 pieceTypeFrom, u8 us, u8 capturePiece){
//     // Update piece set
//     Pos->Board->pieceSet[fromSqr] = Pos->Board->pieceSet[toSqr];
//     Pos->Board->pieceSet[toSqr] = capturePiece;

//     // Move the moving piece, update piece and color bb
//     Pos->Board->colorBB[us] ^= sqrs(fromSqr) | sqrs(toSqr);
//     Pos->Board->piecesBB[pieceTypeFrom] ^= sqrs(fromSqr) | sqrs(toSqr);
// }

// static inline void undo_promote(S_Position* Pos, u8 piece, u8 promotion_piece, u8 from_sqr, u8 to_sqr, u8 ourColor){
//     /*  At the start of make_move function we move the piece without thinking about promotions/
//         Promotions are rather rare, so we just clear the piece from the piece bb    */

//     Pos->Board->piecesBB[p] ^= sqrs(to_sqr);
//     unmove_piece(Pos, from_sqr, to_sqr, p, ourColor, NO_PIECE);

//     Pos->Board->pieceSet[to_sqr] = NO_PIECE;
//     Pos->Board->pieceSet[from_sqr] = p;
//     Pos->Board->piecesBB[promotion_piece] ^= sqrs(to_sqr);
// }

// static inline void undo_promote_capture(S_Position* Pos, u8 piece, u8 promotion_piece, u8 from_sqr, u8 to_sqr, u8 capturePieceType, u8 enemyColor){
//     undo_promote(Pos, piece, promotion_piece, from_sqr, to_sqr, 1 - enemyColor);

//     Pos->Board->piecesBB[capturePieceType] ^= sqrs(to_sqr);
//     Pos->Board->colorBB[enemyColor] ^= sqrs(to_sqr);
//     Pos->Board->pieceSet[to_sqr] = capturePieceType;
// }