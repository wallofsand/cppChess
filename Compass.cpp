#include "Compass.h"

// default constructor.
Compass::Compass() {
    // init stuff
}

Compass::~Compass() { }

ULL* Compass::compute_edge_distances()
{
    ULL arr[64];
    for (int sq = 0; sq < 64; sq++)
    {
        if 
    }
    return arr;
}

ULL* Compass::compute_knight_moves()
{
    ULL arr[64];

    return arr;
}

int Compass::rank(int sq)
{
    return sq >> 3;
}

int Compass::file(int sq)
{
    return sq & 3;
}