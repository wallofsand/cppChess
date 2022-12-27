#include "Bitboard.h"
#include <string>
#include <iostream>

bool contains_square(ULL bb, int sq)
{
    return bb & (1 << sq) != 0;
}

void print_binary_string(ULL bb)
{
    int leading_zero_count = leading_zeros(bb);
    std::string zeros = "";
    for (int i = 0; i < leading_zero_count; i++)
        zeros.append("0");
    while (bb)
    {
        zeros.insert(leading_zero_count, std::to_string(bb&1));
        bb = bb >> 1;
    }
    for (int sq = 63; sq >= 0; sq--)
    {
        std::cout << zeros[sq] << " ";
        if (sq % 8 == 0)
            std::cout << std::endl;
    }
}

int leading_zeros(ULL num)
{
    int count = 0;
    ULL msb = 1ull << 63;
    for (int i = 0; i < 64; i++)
    {
        if (msb >> count & num)
            break;
        count++;
    }
    return count;
}
