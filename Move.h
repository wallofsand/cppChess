#ifndef MOVE_H
#define MOVE_H

#include "Compass.h"
#include <iostream>

typedef uint16_t move;
#define MAXMOVES 120

namespace Move
{
    move build_move(const int s, const int e, const int p = 0);
    int start(const move& m);
    int end(const move& m);
    int promote(const move& m);
    void arr_shift_right(move (&arr)[120], int pos);
    std::string to_string(const move m);
};

#endif
