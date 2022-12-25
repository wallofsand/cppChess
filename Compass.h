#ifndef COMPASS_H
#define COMPASS_H

#define ULL unsigned long long

class Compass
{
public:
    Compass();
    ~Compass();
    ULL* compute_edge_distances();
    ULL* compute_knight_moves();
    int rank(int sq);
    int file (int sq);
private:
};

#endif
