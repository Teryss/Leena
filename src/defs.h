#ifndef DEFS_H
#define DEFS_H

#include <stdint.h>
#include <x86intrin.h>

#define PURE __attribute__((pure))
#define CONST __attribute__((const))
#define INLINE __attribute__((always_inline)) inline

#define SET_BIT(bb, n) (bb |= (1ULL << n))
#define GET_BIT(bb, n) (bb & (1ULL << n))
#define CLEAR_BIT(bb, n) (bb &= ~(1ULL << n))
#define CLEAR_LEAST_SIGNIFICANT_BIT(bb) (bb &= (bb - 1))
#define MORE_THAN_ONE(bb) (bb & (bb - 1))
#define COUNT_BITS(bb) (_popcnt64(bb))
#define PEXT(src, mask) (_pext_u64(src, mask))
#define GET_LEAST_SIGNIFICANT_BIT_INDEX(bb) (_tzcnt_u64(bb))

#define ROW_COL_TO_SQR(row, col) (row * 8 + col)
#define MAX_GAME_SIZE 2048

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
enum load_fen_result {load_fen_success, load_fen_failure};
enum move_type{
    QUIET, DOUBLE_PUSH, KING_CASTLE, QUEEN_CASTLE,
    PROMOTION_N, PROMOTION_B, PROMOTION_R, PROMOTION_Q,
    CAPTURE, EP_CAPTURE,
    CAPTURE_PROMOTION_N, CAPTURE_PROMOTION_B, CAPTURE_PROMOTION_R, CAPTURE_PROMOTION_Q
};
typedef enum __attribute__((__packed__)){
    NODE_NONE, NODE_EXACT, NODE_UPPER, NODE_LOWER
}Node;

typedef struct{
    u64 pieces[12];
    u64 occupied_squares_by[3];
    u64 hash;
    u32 killer_moves[2];
    uint ply;
    u8 enPassantSquare;
    u8 castlePermission;
    u8 fiftyMovesCounter;
    u8 sideToMove;
}S_Board;

typedef struct{
    u64 attacks[107647];
    u64 pawn_attacks[2][64];
    u64 bishop[64];
    u64 queen[64];
    u64 king[64];
    u64 knight[64];
    u64 rook[64];
}S_Masks;

typedef struct{
    u32 moves[256];
    uint count;
}S_Moves;

typedef struct{
    u32 move;
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
    int count;
}S_TTable;

// main.c
extern S_Masks Masks;
void init_all();

// board.c
extern uint load_fen(S_Board * board, const char* const FEN);
extern void reset(S_Board * board);
extern void print_board(S_Board * board);
extern void print_bitboard(u64 bitboard, int current_pos);
extern void update_occupied_squares(S_Board* board); 

extern const char* pieces_int_to_chr;
extern const int piece_chr_to_int[];
extern const char squares_int_to_chr[65][3];
extern const int castling_permission_by_square[64];

// masks.c
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

// constants.c
extern const int16_t PIECE_VALUE[6];
extern const uint BISHOP_PEXT_OFFSET[64];
extern const uint ROOK_PEXT_OFFSET[64];
extern const uint ROOK_RELEVANT_BITS_BY_SQUARE[64];
extern const uint BISHOP_RELEVANT_BITS_BY_SQUARE[64];
extern const uint CASTLING_CHANGE_ON_MOVE[64];
extern const uint MVV_LVA[12][12];
extern const int16_t* PIECE_SQUARE_BONUS[6];

// movegen.c
#define GET_PIECE_FAILED_FLAG 15
#define MOVE_MASK_PIECE (0b1111UL)
#define MOVE_MASK_FROM_SQUARE (0b111111UL << 4UL)
#define MOVE_MASK_TO_SQUARE (0b111111UL << 10UL)
#define MOVE_MASK_PROMOTION_PIECE (0b1111UL << 16UL)
#define MOVE_MASK_CAPTURE_PIECE (0b1111UL << 20UL)
#define MOVE_MASK_FLAG (0b1111UL << 24UL)
#define MOVE_GET_PIECE(move) (move & MOVE_MASK_PIECE)
#define MOVE_GET_FROM_SQUARE(move) ((move & MOVE_MASK_FROM_SQUARE) >> 4)
#define MOVE_GET_TO_SQUARE(move) ((move & MOVE_MASK_TO_SQUARE) >> 10)
#define MOVE_GET_PROMOTION_PIECE(move) ((move & MOVE_MASK_PROMOTION_PIECE) >> 16)
#define MOVE_GET_CAPTURE_PIECE(move) ((move & MOVE_MASK_CAPTURE_PIECE) >> 20)
#define MOVE_GET_FLAG(move) ((move & MOVE_MASK_FLAG) >> 24)

extern void generateMoves(const S_Board* const board, S_Moves* Moves);
extern void generateOnlyCaptures(const S_Board* const board, S_Moves* Moves);
extern CONST u16 encode_state(const S_Board* const Board);
extern CONST u32 encode_move(u8 piece, u8 from_square, u8 to_square, u8 promotion_piece, u8 capture_piece, u8 move_flag);
extern void print_moves(S_Moves* Moves);
extern void print_move(u32 move);
extern PURE u8 is_king_attacked(const S_Board* const board);

// perft.c
extern void perft_suite(S_Board* Board);
extern u64 perft(S_Board* Board, uint depth);
extern void make_move(S_Board* board, u32 move);

// search.c
extern u64 total_nodes_searched;
extern S_Move search(S_Board* Board, uint depth);

// eval.c
extern u32 killer_moves[MAX_GAME_SIZE][2];
extern i32 evaluate(S_Board * Board);
extern void clear_killer_moves();
extern void sort_moves(S_Board* Board, S_Moves* Moves);
extern void sort_captures(S_Board* Board, S_Moves* Moves);

// ttable.c
extern S_TTable TTable;
extern u64 TT_squares_hash[12][64];
extern u64 TT_castling_rights_hash[16];
extern u64 TT_enpassant_hash[8];
extern u64 TT_side_to_move_hash;
extern void init_TT();
extern void hash_position(S_Board* Board);

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

#endif