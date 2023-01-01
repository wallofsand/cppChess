#ifndef BITBOARD_H
#define BITBOARD_H

#include <string>
#include <iostream>

#define U64 uint64_t

namespace Bitboard
{
    const U64 NOTAFILE = 0xfefefefefefefefe;
    const U64 NOTHFILE = 0x7f7f7f7f7f7f7f7f;
    U64 nort_shift(U64 bb);
    U64 sout_shift(U64 bb);
    U64 east_shift(U64 bb);
    U64 west_shift(U64 bb);
    U64 NoEa_shift(U64 bb);
    U64 NoWe_shift(U64 bb);
    U64 SoEa_shift(U64 bb);
    U64 SoWe_shift(U64 bb);
    U64 nort_occl(U64 gen, U64 pro);
    U64 sout_occl(U64 gen, U64 pro);
    U64 east_occl(U64 gen, U64 pro);
    U64 west_occl(U64 gen, U64 pro);
    U64 NoEa_occl(U64 gen, U64 pro);
    U64 NoWe_occl(U64 gen, U64 pro);
    U64 SoEa_occl(U64 gen, U64 pro);
    U64 SoWe_occl(U64 gen, U64 pro);
    bool contains_square(U64 bb, int sq);
    int num_bits_flipped(U64 bb);
    std::string build_binary_string(U64 bb);
    void print_binary_string(std::string bbstr, bool fmt = true);
    int leading_zeros(U64 num);
}

#endif
