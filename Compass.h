#ifndef COMPASS_H
#define COMPASS_H

#include <cstdint>

#define ULL unsigned long long

class Compass
{
public:
    int NORTH = 8, EAST = 1, SOUTH = -8, WEST = -1, NORTHEAST = 9, NORTHWEST = 7, SOUTHWEST = -9,
        SOUTHEAST = -7, NNE = 17, NEE = 10, SEE = -6, SSE = -15, SSW = -17, SWW = -10, NWW = 6, NNW = 15;
    int directions[16] = { NORTH, EAST, SOUTH, WEST,
        NORTHEAST, NORTHWEST, SOUTHWEST, SOUTHEAST,
        NNE, NEE, SEE, SSE, SSW, SWW, NWW, NNW };
    ULL knight_moves[64];
    ULL king_attacks[64];
    uint8_t rook_rows[(8 << 6) | (7 << 3) | 6];
    int rdir[4];
    int bdir[4];
    int pdir[2];
    int whitepawnattacks[2];
    int blackpawnattacks[2];
    int* pattacks[2];
    int edge_distance64x8[64][8];
    Compass();
    ~Compass();
    void init_directions();
    void compute_edge_distances();
    void compute_knight_moves();
    void compute_rook_moves();
    void compute_king_attacks();
    int rank_index(int sq);
    int file_index(int sq);
private:
};

#endif
