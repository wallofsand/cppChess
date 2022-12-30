#include "Move.h"

Move::Move(int s, int e)
{
    start = s;
    end = e;
    flag = get_flag(start, end);
}

Move::~Move() { }

int Move::get_flag(int start, int end)
{
    int flag = 0;
    return flag;
}
