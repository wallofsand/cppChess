#include "Bitboard.h"

bool Bitboard::contains_square(U64 bb, int sq)
{
    return (bb & 1ull << sq) != 0;
}

int Bitboard::num_bits_flipped(U64 bb)
{
    int count = 0;
    for (int i = 0; i < 64; i++)
    {
        count += contains_square(bb, i) ? 1 : 0;
    }
    return count;
}

std::string Bitboard::build_binary_string(U64 bb)
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

void Bitboard::print_binary_string(std::string bbstr, bool fmt)
{
    // we want to print the binary as 8 8-bit words in reverse order
    std::string divider = "!---!---!---!---!---!---!---!---!";
    for (int file = 7; file >= 0; file--)
    {
        if (fmt)
            std::cout << divider << std::endl << "!";
        for (int rank = 0; rank < 8; rank++)
        {
            std::cout << " " << bbstr[(file<<3) + rank] << " ";
            if (fmt)
                std::cout << "!";
        }
        std::cout << std::endl;
    }
    if (fmt)
        std::cout << divider << std::endl;
}

int Bitboard::leading_zeros(U64 num)
{
    int count = 0;
    U64 msb = 1ull << 63;
    for (int i = 0; i < 64; i++)
    {
        if (msb >> count & num)
            break;
        count++;
    }
    return count;
}
