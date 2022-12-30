#ifndef COMPASS_H
#define COMPASS_H

#include <cstdint>
#define U64 uint64_t

namespace directions {
    const int NORTH = 8, EAST = 1, SOUTH = -8, WEST = -1,
        NORTHEAST = 9, NORTHWEST = 7, SOUTHEAST = -9, SOUTHWEST = -7,
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

    Compass();

    static void init_compass();
    static U64 build_ray(int sq, int dir_index);
    static U64 build_ray(int* sq);
    static void compute_edge_distances();
    static void compute_knight_attacks();
    static void compute_rook_attacks();
    static void compute_king_attacks();
    static int get_dir_start_index(int piece);
    static int get_dir_end_index(int piece);
    static int rank_yindex(int sq);
    static int file_xindex(int sq);
};

#endif
