#include "Move.h"

move Move::build_move(int s, int e, int p)
{
    return p << 12 | e << 6 | s;
}

const int Move::start(move m)
{
    return m & 63;
}

const int Move::end(move m)
{
    return m >> 6 & 63;
}

const int Move::promote(move m)
{
    return m >> 12;
}

const std::string Move::to_string(move m)
{
    return Compass::string_from_square(start(m)) + Compass::string_from_square(end(m))
        + (promote(m) ? "=" + ch_cst::piece_char[promote(m)] : "");
}
