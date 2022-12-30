#ifndef BITBOARD_H
#define BITBOARD_H

#include <string>
#include <iostream>

#define U64 uint64_t

namespace Bitboard
{
    bool contains_square(U64 bb, int sq);
    int num_bits_flipped(U64 bb);
    std::string build_binary_string(U64 bb);
    void print_binary_string(std::string bbstr, bool fmt = true);
    int leading_zeros(U64 num);
}

#endif
