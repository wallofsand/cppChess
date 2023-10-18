#include "Move.h"

move Move::build_move(const int s, const int e, const int p) {
    return p << 12 | e << 6 | s;
}

int Move::start(const move& m) {
    return m & 63;
}

int Move::end(const move& m) {
    return (m >> 6) & 63;
}

int Move::promote(const move& m) {
    return m >> 12;
}

std::string Move::to_string(const move m) {
    return ch_cst::string_from_square[start(m)]+ ch_cst::string_from_square[end(m)]
        + (promote(m) ? "=" + ch_cst::piece_char[promote(m)] : "");
}

/*
 * Method to shift each element before pos in an array to the right.
 * The first element of the array is replaced with the element at pos.
 */
void Move::arr_shift_right(move (&arr)[MAXMOVES], int pos) {
    move best = arr[pos];
    while (pos > 0) {
        arr[pos] = arr[pos - 1];
        pos--;
    }
    arr[0] = best;
}
