#ifndef MOVE_H
#define MOVE_H

#include <iostream>

class Move
{
public:
    Move(int s, int e);
    Move(int s, int e, int type);
    int start;
    int end;
    int promote;
    friend std::ostream& operator<<(std::ostream& outS, const Move& mv);
private:
};

#endif
