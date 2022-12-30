#ifndef COMPASS_H
#define COMPASS_H

#include <cstdint>
#define ULL unsigned long long

class Compass
{
public:
    const int NORTH = 8, EAST = 1, SOUTH = -8, WEST = -1,
        NORTHEAST = 9, NORTHWEST = 7, SOUTHWEST = -9, SOUTHEAST = -7,
        NNE = 17, NEE = 10, SEE = -6, SSE = -15,
        SSW = -17, SWW = -10, NWW = 6, NNW = 15;
    const int directions[16] = { NORTH, EAST, SOUTH, WEST,
                          NORTHEAST, NORTHWEST, SOUTHWEST, SOUTHEAST,
                          NNE, NEE, SEE, SSE, SSW, SWW, NWW, NNW};
    const int pawn_moves[2] = { NORTH, SOUTH };
    static ULL KNIGHT_ATTACKS[64];
    static ULL KING_ATTACKS[64];
    // indexed by [(l << 6) | (rksq << 3) | r]
    static uint8_t ROOK_ROWS[(8 << 6) | (7 << 3) | 6];
    static int edge_distance64x8[64][8];

    Compass();

    ULL get_ray(int sq, int* dir);
    ULL get_ray(int sq0, int sq1);
    void compute_edge_distances();
    void compute_knight_attacks();
    void compute_rook_attacks();
    void compute_king_attacks();
    static uint8_t get_rook_row(int rksq, int lsq, int rsq);
    static int get_dir_start_index(int piece);
    static int get_dir_end_index(int piece);
    static int rank_index(int sq);
    static int file_index(int sq);
};

#endif
