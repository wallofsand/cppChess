#include "Move.h"
#include "Compass.h"

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

std::ostream& operator<<(std::ostream& outS, const Move& mv)
{
    return outS << Compass::string_from_square(mv.start) << Compass::string_from_square(mv.end);
}
