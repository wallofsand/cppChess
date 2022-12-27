#include "Compass.h"

// default constructor.
Compass::Compass() {
    // init stuff
    compute_edge_distances();
    compute_knight_moves();
}

Compass::~Compass() { }

int NORTH = 8, EAST = 1, SOUTH = -8, WEST = -1, NORTHEAST = 9, NORTHWEST = 7, SOUTHWEST = -9,
    SOUTHEAST = -7, NNE = 17, NEE = 10, SEE = -6, SSE = -15, SSW = -17, SWW = -10, NWW = 6, NNW = 15;
int directions[] = { NORTH, EAST, SOUTH, WEST, NORTHEAST, NORTHWEST, SOUTHWEST, SOUTHEAST, NNE,
    NEE, SEE, SSE, SSW, SWW, NWW, NNW };
int rdir[] = { SOUTH, WEST, EAST, NORTH };
int bdir[] = { SOUTHWEST, SOUTHEAST, NORTHWEST, NORTHEAST };
int pdir[] = { NORTH, SOUTH };
int whitepawnattacks[] = { NORTHEAST, NORTHWEST };
int blackpawnattacks[] = { SOUTHEAST, SOUTHWEST };
int* pattacks[] = { whitepawnattacks, blackpawnattacks };

void Compass::compute_edge_distances()
{
    for (int sq = 0; sq < 64; sq++)
    {
        int rank = rank_index(sq);
        int file = file_index(sq);
        int nstep = 7 - rank;
        int estep = 7 - file;
        int sstep = rank;
        int wstep = file;
        edge_distance64x8[sq][0] = nstep;
        edge_distance64x8[sq][1] = estep;
        edge_distance64x8[sq][2] = sstep;
        edge_distance64x8[sq][3] = wstep;
        edge_distance64x8[sq][4] = nstep < estep ? nstep : estep;
        edge_distance64x8[sq][5] = nstep < wstep ? nstep : wstep;
        edge_distance64x8[sq][6] = sstep < wstep ? sstep : wstep;
        edge_distance64x8[sq][7] = sstep < estep ? sstep : estep;
    }
}

void Compass::compute_knight_moves()
{
    for (int sq = 0; sq < 64; sq++)
    {
        // NORTH, EAST, SOUTH, WEST
        // NNE, NEE, SEE, SSE, SSW, SWW, NWW, NNW;
        knight_moves[sq] = 0b0;
        if (edge_distance64x8[sq][0] > 0)
        {
            if (edge_distance64x8[sq][0] > 1 && edge_distance64x8[sq][1] > 0)
                knight_moves[sq] |= 1ull << (sq + NNE);
            if (edge_distance64x8[sq][1] > 1)
                knight_moves[sq] |= 1ull << (sq + NEE);
            if (edge_distance64x8[sq][0] > 1 && edge_distance64x8[sq][3] > 0)
                knight_moves[sq] |= 1ull << (sq + NNW);
            if (edge_distance64x8[sq][3] > 1)
                knight_moves[sq] |= 1ull << (sq + NWW);
        }
        if (edge_distance64x8[sq][2] > 0)
        {
            if (edge_distance64x8[sq][2] > 1 && edge_distance64x8[sq][1] > 0)
                knight_moves[sq] |= 1ull << (sq + SSE);
            if (edge_distance64x8[sq][1] > 1)
                knight_moves[sq] |= 1ull << (sq + SEE);
            if (edge_distance64x8[sq][2] > 1 && edge_distance64x8[sq][3] > 0)
                knight_moves[sq] |= 1ull << (sq + SSW);
            if (edge_distance64x8[sq][3] > 1)
                knight_moves[sq] |= 1ull << (sq + SWW);
        }
    }
}

int Compass::rank_index(int sq)
{
    return sq >> 3;
}

int Compass::file_index(int sq)
{
    return sq & 7;
}