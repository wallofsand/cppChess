#ifndef BITBOARD_H
#define BITBOARD_H

#include <string>
#include <iostream>
#include <intrin.h>

#define U64 uint64_t

// __cpuid intrinsic with InfoType=0x80000001

namespace BB
{
    // intersect before west (right) shifts
    const U64 NOT_A_FILE = 0xfefefefefefefefe;
    // intersect before east (left) shifts
    const U64 NOT_H_FILE = 0x7f7f7f7f7f7f7f7f;
    U64 nort_shift_one(U64 bb);
    U64 sout_shift_one(U64 bb);
    U64 east_shift_one(U64 bb);
    U64 west_shift_one(U64 bb);
    U64 NoEa_shift_one(U64 bb);
    U64 NoWe_shift_one(U64 bb);
    U64 SoEa_shift_one(U64 bb);
    U64 SoWe_shift_one(U64 bb);
    U64 nort_occl_fill(U64 gen, U64 empty);
    U64 sout_occl_fill(U64 gen, U64 empty);
    U64 east_occl_fill(U64 gen, U64 empty);
    U64 west_occl_fill(U64 gen, U64 empty);
    U64 NoEa_occl_fill(U64 gen, U64 empty);
    U64 NoWe_occl_fill(U64 gen, U64 empty);
    U64 SoEa_occl_fill(U64 gen, U64 empty);
    U64 SoWe_occl_fill(U64 gen, U64 empty);
    U64 nort_attacks(U64 rooks, U64 empty);
    U64 sout_attacks(U64 rooks, U64 empty);
    U64 east_attacks(U64 rooks, U64 empty);
    U64 west_attacks(U64 rooks, U64 empty);
    U64 NoEa_attacks(U64 bishops, U64 empty);
    U64 NoWe_attacks(U64 bishops, U64 empty);
    U64 SoEa_attacks(U64 bishops, U64 empty);
    U64 SoWe_attacks(U64 bishops, U64 empty);
    U64 gen_shift(U64 bb, int s);
    U64 flip_vertical(U64 bb);
    const bool contains_square(U64 bb, int sq);
    const int num_bits_flipped(U64 bb);
    const void print_U64(U64 bb, std::string name = "", bool fmt = false);
    const void print_binary_string(std::string bbstr, bool fmt = false);
    const std::string build_binary_string(U64 bb);
    const int lead_0s(U64 bb);
}

#endif
