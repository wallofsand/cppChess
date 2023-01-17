#ifndef MOVE_H
#define MOVE_H

#include "Compass.h"
#include <iostream>

typedef uint32_t move;

namespace Move
{
    move build_move(int s, int e, int p = 0);
    const int start(move m);
    const int end(move m);
    const int promote(move m);
    const std::string to_string(move m);
};

#endif
