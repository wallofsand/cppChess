#include "Move.h"

move Move::build_move(uint8_t s, uint8_t e, uint8_t p)
{
    return p << 12 | e << 6 | s;
}

uint8_t Move::start(move m)
{
    return m & 63;
}

uint8_t Move::end(move m)
{
    return (m >> 6) & 63;
}

uint8_t Move::promote(move m)
{
    return m >> 12;
}

std::string Move::to_string(move m)
{
    return Compass::string_from_square(start(m)) + Compass::string_from_square(end(m))
        + (promote(m) ? "=" + ch_cst::piece_char[promote(m)] : "");
}
