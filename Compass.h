#ifndef COMPASS_H
#define COMPASS_H

#include "Bitboard.h"
#include <iostream>
#include <string>

#define U64 uint64_t

namespace ch_cst
{
    const std::string square_string[64] = {
        "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
        "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
        "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
        "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
        "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
        "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
        "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
        "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8"
    };
    /*
     * Indexed by [color << 3 | piece] where
     *      color: (0 - 1) white, black
     *      piece: (1 - 6) pawn - king
     */
    const char piece_char[15] = {
        ' ', 'P', 'N', 'B', 'R', 'Q', 'K', '.',
        '.', 'o', 'n', 'b', 'r', 'q', 'k'
    };
    enum Square {
        a1, b1, c1, d1, e1, f1, g1, h1,
        a2, b2, c2, d2, e2, f2, g2, h2,
        a3, b3, c3, d3, e3, f3, g3, h3,
        a4, b4, c4, d4, e4, f4, g4, h4,
        a5, b5, c5, d5, e5, f5, g5, h5,
        a6, b6, c6, d6, e6, f6, g6, h6,
        a7, b7, c7, d7, e7, f7, g7, h7,
        a8, b8, c8, d8, e8, f8, g8, h8
    };
    const uint8_t PAWN = 1;
    const uint8_t KNIGHT = 2;
    const uint8_t BISHOP = 3;
    const uint8_t ROOK = 4;
    const uint8_t QUEEN = 5;
    const uint8_t KING = 6;
    const uint8_t WHITE_INDEX = 0;
    const uint8_t BLACK_INDEX = 1;
}

namespace directions {
    const int8_t NORTH = 8, EAST = 1, SOUTH = -8, WEST = -1,
            NORTHEAST = 9, NORTHWEST = 7, SOUTHEAST = -7, SOUTHWEST = -9,
            NNE = 17, NEE = 10, SEE = -6, SSE = -15,
            SSW = -17, SWW = -10, NWW = 6, NNW = 15;
    const int8_t DIRS[16] = { NORTH, EAST, SOUTH, WEST,
            NORTHEAST, NORTHWEST, SOUTHEAST, SOUTHWEST,
            NNE, NEE, SEE, SSE, SSW, SWW, NWW, NNW};
    const int8_t PAWN_DIR[2] = { NORTH, SOUTH };
}

class Compass
{
public:
    Compass();
    static U64 knight_attacks[64];
    static U64 king_attacks[64];
    static uint8_t edge_distance_64x8[64][8];
    static uint8_t first_rank_attacks_64x8[64*8]; // 64 * 8 = 512 Bytes = 1/2 KByte
    const static U64 rank_attacks(U64 occ, int sq);
    const static U64 build_ray(uint8_t sq, uint8_t dir_index);
    const static U64 build_ray(uint8_t sq[2]);
    const static U64 ray_square(uint8_t start, uint8_t end, U64 occ = 0ull);
    const static uint8_t get_dir_start_index(uint8_t piece);
    const static uint8_t get_dir_end_index(uint8_t piece);
    const static std::string string_from_square(int8_t sq);
    const static int8_t square_from_string(std::string str);
    const static int8_t rank_yindex(int8_t sq);
    const static int8_t file_xindex(int8_t sq);
private:
    static void compute_edge_distances();
    static void compute_knight_attacks();
    static void compute_rank_attacks();
    static void compute_king_attacks();
};

#endif
