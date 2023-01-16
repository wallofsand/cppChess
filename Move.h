#ifndef MOVE_H
#define MOVE_H

#include "Compass.h"
#include <iostream>

class Move
{
public:
    Move(int s, int e);
    Move(int s, int e, int p);
    Move(const Move& m);

    uint32_t data;

    const friend bool operator!=(Move& m0, Move& m1);
    const friend std::ostream& operator<<(std::ostream& outS, Move& mv);
    const int start();
    const int end();
    const int promote();
    const std::string to_string();
};

#endif
