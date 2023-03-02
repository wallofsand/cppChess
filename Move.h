#ifndef MOVE_H
#define MOVE_H

#include "Compass.h"
#include <iostream>

typedef uint16_t move;

namespace Move
{
    move build_move(const int s, const int e, const int p = 0);
    int start(const move& m);
    int end(const move& m);
    int promote(const move& m);
    std::string to_string(const move m);
};

#endif
