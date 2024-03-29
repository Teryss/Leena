#ifndef DEFS_H
#define DEFS_H

#include <stdint.h>
#include <x86intrin.h>

// #define DEBUG

#ifndef DEBUG
#define ASSERT(n)
#else
#define ASSERT(n) \
if(!(n)) { \
printf("%s - Failed ",#n); \
printf("On %s ",__DATE__); \
printf("At %s ",__TIME__); \
printf("In File %s ",__FILE__); \
printf("At Line %d\n",__LINE__); \
exit(1);}
#endif


#define PURE __attribute__((pure))
#define CONST __attribute__((const))
#define INLINE __attribute__((always_inline)) inline

#define NOINLINE __attribute__((noinline))

#define SET_BIT(bb, n) (bb |= (1ULL << n))
#define GET_BIT(bb, n) (bb & (1ULL << n))
#define CLEAR_BIT(bb, n) (bb &= ~(1ULL << n))
#define CLEAR_LEAST_SIGNIFICANT_BIT(bb) (bb = _blsr_u64(bb))
#define MORE_THAN_ONE(bb) (_blsr_u64(bb))
#define COUNT_BITS(bb) (_popcnt64(bb))
#define PEXT(src, mask) (_pext_u64(src, mask))
#define GET_LEAST_SIGNIFICANT_BIT_INDEX(bb) (_tzcnt_u64(bb))
// #define sqrs(sqr) (1ULL << (sqr))
#define sqrs(sqr) (sqrs[sqr])

#define ROW_COL_TO_SQR(row, col) (row * 8 + col)
#define MAX_GAME_SIZE 2048
#define MAX_SEARCH_DEPTH 64
#define MB 1024
#define TT_SIZE 100 * MB

#define STARTING_POSITION_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define kiwipete "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"

typedef uint64_t u64;
typedef uint32_t u32;
typedef unsigned int uint;
typedef uint16_t u16;
typedef uint8_t u8;
typedef int32_t i32;

enum squares{
    A8, B8, C8, D8, E8, F8, G8, H8,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A1, B1, C1, D1, E1, F1, G1, H1, NO_SQR,
};
enum pieces {p, n, b, r, q, k, P, N, B, R, Q, K, NO_PIECE};
enum color {WHITE, BLACK, BOTH};
enum castling {wk = 1, wq = 2, bk = 4, bq = 8};
enum load_fen_result {SUCCESS, TOO_MANY_SPACES, NOT_ENOUGH_INFO, WRONG_SIDE_TO_MOVE, WRONG_CASTLE_PERM, WRONG_EN_PASSANT};
enum move_type{
    QUIET, DOUBLE_PUSH, KING_CASTLE, QUEEN_CASTLE,
    PROMOTION_N, PROMOTION_B, PROMOTION_R, PROMOTION_Q,
    CAPTURE, EP_CAPTURE,
    CAPTURE_PROMOTION_N, CAPTURE_PROMOTION_B, CAPTURE_PROMOTION_R, CAPTURE_PROMOTION_Q
};
typedef enum { GENERATE_ALL, GENERATE_CAPTURES } GenType;
typedef enum __attribute__((__packed__)){
    NODE_NONE, NODE_EXACT, NODE_UPPER, NODE_LOWER
}Node;

typedef struct{
    u64 piecesBB[6];
    u64 colorBB[3];
    u64 hash;
    u8 pieceSet[64];
}S_Board;

typedef struct{
    u16 nodes[MAX_SEARCH_DEPTH][MAX_SEARCH_DEPTH];
    u8 length[MAX_SEARCH_DEPTH];
}S_PV;

typedef struct{
    S_PV PV;
    uint ply;
    S_Board* Board;
    u8 enPassantSquare;
    u8 castlePermission;
    u8 fiftyMovesCounter;
    u8 sideToMove;
}S_Position;

typedef struct{
    u64 attacks[107647];
    u64 pawn_attacks[2][64];
    u64 bishop[64];
    u64 king[64];
    u64 knight[64];
    u64 rook[64];
}S_Masks;

typedef struct{
    u16 moves[256];
    uint count;
}S_Moves;

typedef struct{
    u16 move;
    i32 score;
}S_Move;

typedef struct{
    u64 hash;
    i32 score;
    Node node_type;
    u8 depth;
}S_TT_Entry;

typedef struct{
    S_TT_Entry* entries;
    uint count;
}S_TTable;

// main.c
extern S_Masks Masks;
void init_all();
extern u64 sqrs[64];

// bitboards.c
extern u64 between[64][64];
extern u64 line[64][64];
extern void init_bb();

// board.c
extern uint load_fen(S_Position* Pos, const char* const FEN);
extern void reset(S_Position* pos);
extern void print_board(const S_Position* Pos);
extern void print_bitboard(u64 bitboard);
extern const char* pieces_int_to_chr;
extern const char squares_int_to_chr[65][3];
extern const int castling_permission_by_square[64];

// masks.c
extern const uint BISHOP_PEXT_OFFSET[64];
extern const uint ROOK_PEXT_OFFSET[64];
extern const uint ROOK_RELEVANT_BITS_BY_SQUARE[64];
extern const uint BISHOP_RELEVANT_BITS_BY_SQUARE[64];
extern const uint CASTLING_CHANGE_ON_MOVE[64];
extern void init_masks();
extern u64 get_blockers(uint index, uint relevant_bits, u64 mask);
extern u64 mask_pawn_attacks(int square, int color);
extern u64 mask_king_attacks(int square);
extern u64 mask_knight_attacks(int square);
extern u64 mask_rook_attacks(int square);
extern u64 mask_rook_attacks_on_the_fly(int square, u64 blockers);
extern u64 mask_bishop_attacks(int square);
extern u64 mask_bishop_attacks_on_the_fly(int square, u64 blockers);
extern u64 mask_queen_attacks(int square);

// movegen.c
#define GET_PIECE_FAILED_FLAG 15
#define MOVE_FROM_SQUARE(move) (move & 0b111111U)
#define MOVE_TO_SQUARE(move) ((move & (0b111111UL << 6UL)) >> 6)
#define MOVE_GET_FLAG(move) ((move & (0b1111UL << 12UL)) >> 12)
extern void generateMoves(const S_Position* const Pos, S_Moves* Moves);
extern void generateOnlyCaptures(const S_Position* const Pos, S_Moves* Moves);
extern CONST u8 is_king_attacked(const S_Position* const Pos);
extern void filter_illegal(const S_Position* const Pos, S_Moves* Moves);

// move.c
#define GET_CASTLE_PERM(state) (state & 0b1111)
#define GET_FIFTY_MOVES_COUNTER(state) ((state >> 4) & 0b111111)
#define GET_EN_PASSANT_SQR(state) ((state >> 10) & 0b111111)
extern void make_move(S_Position* Pos, u16 move);
extern void undo_move(S_Position* Pos, u16 move, u16 lastState, u8 capturePiece);
extern void print_moves(S_Board* Board, S_Moves* Moves);
extern void print_move(S_Board* Board, u16 move);

// perft.c
extern void perft_suite(S_Position* Pos);
extern u64 perft(S_Position* Pos, uint depth);

// eval.c
extern u32 killer_moves[MAX_GAME_SIZE][2];
extern i32 evaluate(S_Position* Pos);
extern void clear_killer_moves();
extern void sort_moves(const S_Position* const Pos, S_Moves* Moves);
extern void sort_captures(const S_Position* const Pos, S_Moves* Moves);

// ttable.c
extern S_TTable TTable;
extern u64 TT_squares_hash[2][6][64];
extern u64 TT_castling_rights_hash[16];
extern u64 TT_enpassant_hash[8];
extern u64 TT_side_to_move_hash;
extern void init_TT();
extern void hash_position(S_Position* Pos);

// uci.c
void uci_loop();

INLINE u64 get_rook_attacks(u64 occupancy, const uint square){
    return Masks.attacks[PEXT(occupancy, Masks.rook[square]) + ROOK_PEXT_OFFSET[square]];
}

INLINE u64 get_bishop_attacks(u64 occupancy, const uint square){
    return Masks.attacks[PEXT(occupancy, Masks.bishop[square]) + BISHOP_PEXT_OFFSET[square]];
}

INLINE u64 get_queen_attacks(u64 occupancy, const uint square){
    return get_bishop_attacks(occupancy, square) | get_rook_attacks(occupancy, square);
}

INLINE u8 pieceChrToInt(char pieceChr){
    switch (pieceChr){
        case 'p':
        case 'P':
            return 0;
        case 'n':
        case 'N':
            return 1;
        case 'b':
        case 'B':
            return 2;
        case 'r':
        case 'R':
            return 3;
        case 'q':
        case 'Q':
            return 4;
        case 'k':
        case 'K':
            return 5;
        default:
            return NO_PIECE;
    }
}

INLINE void restore_state(S_Position* Pos, u16 state){
    Pos->enPassantSquare = GET_EN_PASSANT_SQR(state);
    Pos->castlePermission = GET_CASTLE_PERM(state);
    Pos->fiftyMovesCounter = GET_FIFTY_MOVES_COUNTER(state);
    Pos->sideToMove = 1 - Pos->sideToMove;
    Pos->ply--;
}

INLINE u16 encode_state(S_Position* Pos){
    return (u16)(
        (Pos->castlePermission & 0b1111) |
        (Pos->fiftyMovesCounter & 0b111111) << 4 |
        Pos->enPassantSquare << 10
    );
}

// INLINE void generate(const S_Position* const Pos, S_Moves* Moves, GenType WhatToGenerate){
//     switch (WhatToGenerate) {
//         case GENERATE_ALL:
//             generateMoves(Pos, Moves);
//             filter_illegal(Pos, Moves);
//             break;
//         case GENERATE_CAPTURES:
//             generateOnlyCaptures(Pos, Moves);
//             filter_illegal(Pos, Moves);
//             break;
//     }
// }

#endif