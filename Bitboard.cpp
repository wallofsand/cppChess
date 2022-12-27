#include "Bitboard.h"


bool contains_square(ULL bb, int sq)
{
    return bb & (1 << sq) != 0;
}

std::string build_binary_string(ULL bb)
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
    // string is currently reversed - the string is the correct binary representation of the number
    // but that means the bits are indexed backwards. lets fix that
    for (int idx = 0; idx < 32; idx++)
    {
        char tmp = zeros[idx];
        zeros[idx] = zeros[63-idx];
        zeros[63-idx] = tmp;
    }
    return zeros;
}

void print_binary_string(std::string bbstr)
{
    // we want to print the binary as 8 8-bit words in reverse order
    for (int file = 7; file >= 0; file--)
    {
        for (int rank = 0; rank < 8; rank++)
        {
            std::cout << bbstr[(file<<3) + rank] << " ";
        }
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
