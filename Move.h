#ifndef MOVE_H
#define MOVE_H

#include "Compass.h"
#include <iostream>

typedef uint32_t move;

namespace Move
{
    move build_move(uint8_t s, uint8_t e, uint8_t p = 0);
    uint8_t start(move m);
    uint8_t end(move m);
    uint8_t promote(move m);
    std::string to_string(move m);
};

#endif
