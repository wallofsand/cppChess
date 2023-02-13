#ifndef MOVE_H
#define MOVE_H

#include "Compass.h"
#include <iostream>

typedef uint16_t move;

namespace Move
{
    move build_move(const uint8_t s, const uint8_t e, const uint8_t p = 0);
    uint8_t start(const move& m);
    uint8_t end(const move& m);
    uint8_t promote(const move& m);
    std::string to_string(const move m);
};

#endif
