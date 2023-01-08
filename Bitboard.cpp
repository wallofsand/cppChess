#include "Bitboard.h"

/*
 * @param bb any bitboard
 * @param sq square index 0-63
 * @return true if the bit at sq is set
 */
bool Bitboard::contains_square(U64 bb, int sq)
{
    return (bb & 1ull << sq) != 0;
}

/*
 * @param bb any bitboard
 * @return the number of bits set in bb
 */
int Bitboard::num_bits_flipped(U64 bb)
{
    int count = 0;
    for (int i = 0; bb >> i; i++)
    {
        count += contains_square(bb, i) * 1;
    }
    return count;
}

/*
 * north shift by 1
 * @param bb any bitboard
 * @return shifted bitboard
 */
U64 Bitboard::nort_shift_one(U64 bb)
{
    return bb << 8;
}

/*
 * south shift by 1
 * @param bb any bitboard
 * @return shifted bitboard
 */
U64 Bitboard::sout_shift_one(U64 bb)
{
    return bb >> 8;
}

/*
 * east shift by 1 without wrapping
 * @param bb any bitboard
 * @return shifted bitboard
 */
U64 Bitboard::east_shift_one(U64 bb)
{
    return (bb & NOT_H_FILE) << 1;
}

/*
 * west shift by 1 without wrapping
 * @param bb any bitboard
 * @return shifted bitboard
 */
U64 Bitboard::west_shift_one(U64 bb)
{
    return (bb & NOT_A_FILE) >> 1;
}

/*
 * northeast shift by 1 without wrapping
 * @param bb any bitboard
 * @return shifted bitboard
 */
U64 Bitboard::NoEa_shift_one(U64 bb)
{
    return (bb & NOT_H_FILE) << 9;
}

/*
 * northwest shift by 1 without wrapping
 * @param bb any bitboard
 * @return shifted bitboard
 */
U64 Bitboard::NoWe_shift_one(U64 bb)
{
    return (bb & NOT_A_FILE) << 7;
}

/*
 * southeast shift by 1 without wrapping
 * @param bb any bitboard
 * @return shifted bitboard
 */
U64 Bitboard::SoEa_shift_one(U64 bb)
{
    return (bb & NOT_H_FILE) >> 7;
}

/*
 * southwest shift by 1 without wrapping
 * @param bb any bitboard
 * @return shifted bitboard
 */
U64 Bitboard::SoWe_shift_one(U64 bb)
{
    return (bb & NOT_A_FILE) >> 9;
}

/*
 * north occluded fill using Kogge-Stone Algorithm
 * @param gen bitboard to "smear"
 * @param empty set of empty bits
 * @return shifted bitboard
 */
U64 Bitboard::nort_occl_fill(U64 gen, U64 empty)
{
    gen |= empty & (gen << 8);
    empty = empty & (empty << 8);
    gen |= empty & (gen << 16);
    empty = empty & (empty << 16);
    gen |= empty & (gen << 32);
    return gen;
}

/*
 * south occluded fill using Kogge-Stone Algorithm
 * @param gen bitboard to "smear"
 * @param empty bitboard of empty squares
 * @return shifted bitboard
 */
U64 Bitboard::sout_occl_fill(U64 gen, U64 empty)
{
    gen |= empty & (gen >> 8);
    empty = empty & (empty >> 8);
    gen |= empty & (gen >> 16);
    empty = empty & (empty >> 16);
    gen |= empty & (gen >> 32);
    return gen;
}

/*
 * east occluded fill using Kogge-Stone Algorithm
 * @param gen bitboard to "smear"
 * @param empty set of empty bits
 * @return shifted bitboard
 */
U64 Bitboard::east_occl_fill(U64 gen, U64 empty)
{
    empty &= NOT_H_FILE;
    gen |= empty & (gen << 1);
    empty = empty & (empty << 1);
    gen |= empty & (gen << 2);
    empty = empty & (empty << 2);
    gen |= empty & (gen << 4);
    return gen;
}

/*
 * west occluded fill using Kogge-Stone Algorithm
 * @param gen bitboard to "smear"
 * @param empty set of empty bits
 * @return shifted bitboard
 */
U64 Bitboard::west_occl_fill(U64 gen, U64 empty)
{
    empty &= NOT_A_FILE;
    gen |= empty & (gen >> 1);
    empty = empty & (empty >> 1);
    gen |= empty & (gen >> 2);
    empty = empty & (empty >> 2);
    gen |= empty & (gen >> 4);
    return gen;
}

/*
 * northeast occluded fill using Kogge-Stone Algorithm
 * @param gen bitboard to "smear"
 * @param empty set of empty bits
 * @return shifted bitboard
 */
U64 Bitboard::NoEa_occl_fill(U64 gen, U64 empty)
{
    empty &= NOT_H_FILE;
    gen |= empty & (gen << 9);
    empty = empty & (empty << 9);
    gen |= empty & (gen << 18);
    empty = empty & (empty << 18);
    gen |= empty & (gen << 36);
    return gen;
}

/*
 * northwest occluded fill using Kogge-Stone Algorithm
 * @param gen bitboard to "smear"
 * @param empty set of empty bits
 * @return shifted bitboard
 */
U64 Bitboard::NoWe_occl_fill(U64 gen, U64 empty)
{
    empty &= NOT_A_FILE;
    gen |= empty & (gen << 7);
    empty = empty & (empty << 7);
    gen |= empty & (gen << 14);
    empty = empty & (empty << 14);
    gen |= empty & (gen << 28);
    return gen;
}

/*
 * southeast occluded fill using Kogge-Stone Algorithm
 * @param gen bitboard to "smear"
 * @param empty set of empty bits
 * @return shifted bitboard
 */
U64 Bitboard::SoEa_occl_fill(U64 gen, U64 empty)
{
    empty &= NOT_H_FILE;
    gen |= empty & (gen >> 7);
    empty = empty & (empty >> 7);
    gen |= empty & (gen >> 14);
    empty = empty & (empty >> 14);
    gen |= empty & (gen >> 28);
    return gen;
}

/*
 * southwest occluded fill using Kogge-Stone Algorithm
 * @param gen bitboard to "smear"
 * @param empty set of empty bits
 * @return shifted bitboard
 */
U64 Bitboard::SoWe_occl_fill(U64 gen, U64 empty)
{
    empty &= NOT_A_FILE;
    gen |= empty & (gen >> 9);
    empty = empty & (empty >> 9);
    gen |= empty & (gen >> 18);
    empty = empty & (empty >> 18);
    gen |= empty & (gen >> 36);
    return gen;
}

U64 Bitboard::nort_attacks(U64 rooks, U64 empty) { return nort_shift_one(nort_occl_fill(rooks, empty)); }
U64 Bitboard::sout_attacks(U64 rooks, U64 empty) { return sout_shift_one(sout_occl_fill(rooks, empty)); }
U64 Bitboard::east_attacks(U64 rooks, U64 empty) { return east_shift_one(east_occl_fill(rooks, empty)); }
U64 Bitboard::west_attacks(U64 rooks, U64 empty) { return west_shift_one(west_occl_fill(rooks, empty)); }
U64 Bitboard::NoEa_attacks(U64 bishops, U64 empty) { return NoEa_shift_one(NoEa_occl_fill(bishops, empty)); }
U64 Bitboard::NoWe_attacks(U64 bishops, U64 empty) { return NoWe_shift_one(NoWe_occl_fill(bishops, empty)); }
U64 Bitboard::SoEa_attacks(U64 bishops, U64 empty) { return SoEa_shift_one(SoEa_occl_fill(bishops, empty)); }
U64 Bitboard::SoWe_attacks(U64 bishops, U64 empty) { return SoWe_shift_one(SoWe_occl_fill(bishops, empty)); }

/*
 * generalized shift
 * @author Gerd Isenberg
 * @param x any bitboard
 * @param s shift amount -64 < s < +64
 *          left if positive
 *          right if negative
 * @return shifted bitboard
 */
U64 Bitboard::gen_shift(U64 bb, int s)
{
    char left = (char) s;
    char right = -((char) (s >> 8) & left);
    return (bb >> right) << (right + left);
}

/*
 * method to build string representation of bitboards
 * Little-Endian, a1 bit is the first char of the string
 * @param bb any bitboard
 * @return a string of the binary representation of bb
 */
std::string Bitboard::build_binary_string(U64 bb)
{
    int leading_zero_count = leading_zeros(bb);
    std::string zeros = "";
    for (int i = 0; i < leading_zero_count; i++)
        zeros.append("0");
    while (bb)
    {
        zeros.insert(leading_zero_count, std::to_string(bb & 1));
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

/*
 * method to print a 64 character string as a chessboard
 * Little-Endian, a1 is the first char of the string
 * @param bbstr any string of length 64
 * @param fmt true to print fancy output
 */
void Bitboard::print_binary_string(std::string bbstr, bool fmt)
{
    // we want to print the binary as 8 8-bit words
    // the words are printed forewards but in reverse order
    // since the printing happens top-to-bottom on the screen
    std::string divider = "!---!---!---!---!---!---!---!---!";
    for (int file = 7; file >= 0; file--)
    {
        if (fmt)
            std::cout << divider << std::endl << "! ";
        for (int rank = 0; rank < 8; rank++)
        {
            std::cout << bbstr[(file<<3) + rank];
            if (fmt)
                std::cout << " ! ";
        }
        std::cout << std::endl;
    }
    if (fmt)
        std::cout << divider << std::endl;
}

/*
 * method to count the number of leading zeros in a bitboard
 * @param any bitboard
 * @return the number of leading zeros
 */
int Bitboard::leading_zeros(U64 bb)
{
    if (!bb) return 64;
    int count = 0;
    // is the msb in the top 32 bits?
    if (!(bb & 0xFFFFFFFF00000000))
    {
        // it wasn't, shift 32 up
        count += 32;
        bb = bb << 32;
    }
    // how about the next 16?
    if (!(bb & 0xFFFF000000000000))
    {
        count += 16;
        bb = bb << 16;
    }
    // how about the next 8?
    if (!(bb & 0xFF00000000000000))
    {
        count += 8;
        bb = bb << 8;
    }
    if (!(bb & 0xF000000000000000))
    {
        count += 4;
        bb = bb << 4;
    }
    if (!(bb & 0xC000000000000000))
    {
        count += 2;
        bb = bb << 2;
    }
    if (!(bb & 0x8000000000000000))
    {
        count += 1;
    }
    return count;
}
