#include "Move.h"
#include "Compass.h"

Move::Move(int start_sq, int end_sq)
{
    data = end_sq << 6 | start_sq;
}

Move::Move(int start_sq, int end_sq, int type)
{
    data = type << 12 | end_sq << 6 | start_sq;
}

const int Move::start()
{
    return data & 63;
}

const int Move::end()
{
    return data >> 6 & 63;
}

const int Move::promote()
{
    return data >> 12;
}

std::ostream& operator<<(std::ostream& outS, Move& mv)
{
    return outS << Compass::string_from_square(mv.start()) << Compass::string_from_square(mv.end());
}
