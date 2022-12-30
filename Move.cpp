#include "Move.h"

Move::Move(int s, int e)
{
    start = s;
    end = e;
    promote = 0;
}

Move::Move(int s, int e, int type)
{
    start = s;
    end = e;
    promote = type;
}

Move::~Move() { }
