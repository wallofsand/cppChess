#ifndef BITBOARD_H
#define BITBOARD_H

#include <string>
#include <iostream>

#define U64 uint64_t

namespace Bitboard
{
    // intersect before west (left) shifts
    // if the shift amount % 8 != 0
    const U64 NOT_A_FILE = 0xfefefefefefefefe;
    // intersect befre east (right) shifts
    // if the shift amount % 8 != 0
    const U64 NOT_H_FILE = 0x7f7f7f7f7f7f7f7f;
    U64 nort_shift_one(U64 bb);
    U64 sout_shift_one(U64 bb);
    U64 east_shift_one(U64 bb);
    U64 west_shift_one(U64 bb);
    U64 NoEa_shift_one(U64 bb);
    U64 NoWe_shift_one(U64 bb);
    U64 SoEa_shift_one(U64 bb);
    U64 SoWe_shift_one(U64 bb);
    U64 nort_occl_fill(U64 gen, U64 pro);
    U64 sout_occl_fill(U64 gen, U64 pro);
    U64 east_occl_fill(U64 gen, U64 pro);
    U64 west_occl_fill(U64 gen, U64 pro);
    U64 NoEa_occl_fill(U64 gen, U64 pro);
    U64 NoWe_occl_fill(U64 gen, U64 pro);
    U64 SoEa_occl_fill(U64 gen, U64 pro);
    U64 SoWe_occl_fill(U64 gen, U64 pro);
    bool contains_square(U64 bb, int sq);
    int num_bits_flipped(U64 bb);
    std::string build_binary_string(U64 bb);
    void print_binary_string(std::string bbstr, bool fmt = true);
    int leading_zeros(U64 num);
}

#endif
