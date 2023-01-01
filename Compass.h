#ifndef COMPASS_H
#define COMPASS_H

#include "Chess.h"
#include <iostream>
#include <string>
#define U64 uint64_t

namespace ch_cst
{
    const std::string square_to_string[] = {
        "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
        "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
        "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
        "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
        "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
        "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
        "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
        "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8"
    };
    enum enumSquare {
        a1, b1, c1, d1, e1, f1, g1, h1,
        a2, b2, c2, d2, e2, f2, g2, h2,
        a3, b3, c3, d3, e3, f3, g3, h3,
        a4, b4, c4, d4, e4, f4, g4, h4,
        a5, b5, c5, d5, e5, f5, g5, h5,
        a6, b6, c6, d6, e6, f6, g6, h6,
        a7, b7, c7, d7, e7, f7, g7, h7,
        a8, b8, c8, d8, e8, f8, g8, h8
    };
    const int PAWN = 1;
    const int KNIGHT = 2;
    const int BISHOP = 3;
    const int ROOK = 4;
    const int QUEEN = 5;
    const int KING = 6;
    const int WHITE_INDEX = 0;
    const int BLACK_INDEX = 1;
}

namespace directions {
    const int NORTH = 8, EAST = 1, SOUTH = -8, WEST = -1,
        NORTHEAST = 9, NORTHWEST = 7, SOUTHEAST = -7, SOUTHWEST = -9,
        NNE = 17, NEE = 10, SEE = -6, SSE = -15,
        SSW = -17, SWW = -10, NWW = 6, NNW = 15;
    const int DIRS[16] = { NORTH, EAST, SOUTH, WEST,
                          NORTHEAST, NORTHWEST, SOUTHEAST, SOUTHWEST,
                          NNE, NEE, SEE, SSE, SSW, SWW, NWW, NNW};
    const int PAWN_DIR[2] = { NORTH, SOUTH };
}

class Compass
{
public:
    static U64 knight_attacks[64];
    static U64 king_attacks[64];
    static uint8_t rook_rows256x8[256][8];
    static int edge_distance64x8[64][8];
    static void init_compass();
    static U64 build_ray(int sq, int dir_index);
    static U64 build_ray(int* sq);
    static void compute_edge_distances();
    static void compute_knight_attacks();
    static void compute_rook_attacks();
    static void compute_king_attacks();
    static int get_dir_start_index(int piece);
    static int get_dir_end_index(int piece);
    static int square_from_string(std::string str);
    static std::string string_from_square(int sq);
    static int rank_yindex(int sq);
    static int file_xindex(int sq);
};

#endif
