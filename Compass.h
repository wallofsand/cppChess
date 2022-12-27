#ifndef COMPASS_H
#define COMPASS_H

#define ULL unsigned long long

class Compass
{
public:
    int NORTH, EAST, SOUTH, WEST, NORTHEAST, NORTHWEST, SOUTHWEST,
        SOUTHEAST, NNE, NEE, SEE, SSE, SSW, SWW, NWW, NNW;
    int directions[16];
    ULL knight_moves[64];
    int rdir[4];
    int bdir[4];
    int pdir[2];
    int whitepawnattacks[2];
    int blackpawnattacks[2];
    int* pattacks[2];
    int edge_distance64x8[64][8];
    Compass();
    ~Compass();
    void compute_edge_distances();
    void compute_knight_moves();
    int rank_index(int sq);
    int file_index(int sq);
private:
};

#endif
