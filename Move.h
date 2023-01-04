#ifndef MOVE_H
#define MOVE_H

#include <iostream>

class Move
{
public:
    Move(int s, int e);
    Move(int s, int e, int type);
    friend std::ostream& operator<<(std::ostream& outS, Move& mv);
    const int start();
    const int end();
    const int promote();
    const std::string to_string();
private:
    int data;
};

#endif
