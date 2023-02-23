#include "Move.h"

move Move::build_move(const uint16_t s, const uint16_t e, const uint16_t p)
{
    return (p & 15) << 12 | (e & 63) << 6 | (s & 63);
}

uint8_t Move::start(const move& m)
{
    return m & 63;
}

uint8_t Move::end(const move& m)
{
    return (m >> 6) & 63;
}

uint8_t Move::promote(const move& m)
{
    return m >> 12;
}

std::string Move::to_string(const move m)
{
    return Compass::string_from_square(start(m)) + Compass::string_from_square(end(m))
        + (promote(m) ? "=" + ch_cst::piece_char[promote(m)] : "");
}
