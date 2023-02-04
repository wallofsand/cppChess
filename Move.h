#ifndef MOVE_H
#define MOVE_H

#include "Compass.h"
#include <iostream>

typedef uint32_t move;

namespace Move
{
    move build_move(int s, int e, int p = 0);
    int start(move m);
    int end(move m);
    int promote(move m);
    std::string to_string(move m);
};

#endif
