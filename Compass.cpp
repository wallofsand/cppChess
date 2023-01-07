#include "Compass.h"

using namespace directions;

U64 Compass::knight_attacks[64];
U64 Compass::king_attacks[64];
uint8_t Compass::rook_rows256x8[256][8];
int Compass::edge_distance64x8[64][8];

void Compass::init_compass()
{
    // init stuff
    compute_edge_distances();
    compute_knight_attacks();
    compute_rook_attacks();
    compute_king_attacks();
}

// Method to return a bitboard of the ray which
// passes through square sq in direction DIRS[dir_index]
const U64 Compass::build_ray(int sq, int dir_index)
{
    U64 ray = 0;
    for (int step = 1; step <= Compass::edge_distance64x8[sq][dir_index]; step++)
    {
        ray |= 1ull << (sq + step * DIRS[dir_index]);
    }
    return ray;
}

// Method to return a bitboard of the ray which
// passes between two colinear squares sq0 to sq1
const U64 Compass::build_ray(int sq[2])
{
    int sq0 = sq[0];
    int sq1 = sq[1];
    int end_index = get_dir_end_index(ch_cst::QUEEN);
    for (int i = 0; i < end_index; i++)
    {
        int dir = DIRS[i];
        for (int step = 1; step <= edge_distance64x8[sq0][i]; step++)
            if (sq0 + step * DIRS[i] == sq1)
            {
                U64 ray = 0;
                int sq = sq0;
                while(sq != sq1)
                {
                    sq += DIRS[i];
                    ray |= 1ull << sq;
                }
                return ray;
            }
    }
    return 0;
}

void Compass::compute_edge_distances()
{
    for (int sq = 0; sq < 64; sq++)
    {
        int rank = rank_yindex(sq);
        int file = file_xindex(sq);
        int nstep = 7 - rank;
        int estep = 7 - file;
        int sstep = rank;
        int wstep = file;
        edge_distance64x8[sq][0] = nstep;
        edge_distance64x8[sq][1] = estep;
        edge_distance64x8[sq][2] = sstep;
        edge_distance64x8[sq][3] = wstep;
        edge_distance64x8[sq][4] = nstep < estep ? nstep : estep;
        edge_distance64x8[sq][5] = nstep < wstep ? nstep : wstep;
        edge_distance64x8[sq][6] = sstep < wstep ? sstep : wstep;
        edge_distance64x8[sq][7] = sstep < estep ? sstep : estep;
    }
}

void Compass::compute_knight_attacks()
{
    for (int sq = 0; sq < 64; sq++)
    {
        // NORTH, EAST, SOUTH, WEST
        // NNE, NEE, SEE, SSE, SSW, SWW, NWW, NNW;
        Compass::knight_attacks[sq] = 0b0;
        if (edge_distance64x8[sq][0] > 0)
        {
            if (edge_distance64x8[sq][0] > 1 && edge_distance64x8[sq][1] > 0)
                Compass::knight_attacks[sq] |= 1ull << (sq + NNE);
            if (edge_distance64x8[sq][1] > 1)
                Compass::knight_attacks[sq] |= 1ull << (sq + NEE);
            if (edge_distance64x8[sq][0] > 1 && edge_distance64x8[sq][3] > 0)
                Compass::knight_attacks[sq] |= 1ull << (sq + NNW);
            if (edge_distance64x8[sq][3] > 1)
                Compass::knight_attacks[sq] |= 1ull << (sq + NWW);
        }
        if (edge_distance64x8[sq][2] > 0)
        {
            if (edge_distance64x8[sq][2] > 1 && edge_distance64x8[sq][1] > 0)
                Compass::knight_attacks[sq] |= 1ull << (sq + SSE);
            if (edge_distance64x8[sq][1] > 1)
                Compass::knight_attacks[sq] |= 1ull << (sq + SEE);
            if (edge_distance64x8[sq][2] > 1 && edge_distance64x8[sq][3] > 0)
                Compass::knight_attacks[sq] |= 1ull << (sq + SSW);
            if (edge_distance64x8[sq][3] > 1)
                Compass::knight_attacks[sq] |= 1ull << (sq + SWW);
        }
    }
}

void Compass::compute_rook_attacks()
{
    for (int idx = 0; idx < 256; idx++)
    {
        for (int r = 0; r < 8; r++)
        {
            // with a rook on r, and occ = to idx,
            // flip the bits the rook can attack
            rook_rows256x8[idx][r] = 0;
            // std::cout << "l: " << l << " r: " << r << " rksq: " << rksq << std::endl;
            // for (int i = 7; i >= 0; i--)
            //     std::cout << (int(rook_rows[idx] >> i) & 1);
            // std::cout << std::endl;
            // for (int i = 0; i < 8 - rksq; i++)
            //     std::cout << " ";
            // std::cout << "^" << std::endl;
        }
    }
}

void Compass::compute_king_attacks()
{
    for (int sq = 0; sq < 64; sq++)
    {
        king_attacks[sq] = 0b0;
        for (int dir_idx = get_dir_start_index(ch_cst::KING); dir_idx < get_dir_end_index(ch_cst::KING); dir_idx++)
        {
            int dir = DIRS[dir_idx];
            if (edge_distance64x8[sq][dir_idx] == 0)
                continue;
            king_attacks[sq] |= 1ull << (sq + dir);
        }
    }
}

const int Compass::get_dir_start_index(int piece)
{
    switch (piece)
    {
    case ch_cst::KNIGHT:
        return 8;
    case ch_cst::BISHOP:
        return 4;
    case ch_cst::ROOK:
    case ch_cst::QUEEN:
    case ch_cst::KING:
        return 0;
    default:
        return -1;
    }
}

const int Compass::get_dir_end_index(int piece)
{
    switch (piece)
    {
    case ch_cst::KNIGHT:
        return 16;
    case ch_cst::ROOK:
        return 4;
    case ch_cst::BISHOP:
    case ch_cst::QUEEN:
    case ch_cst::KING:
        return 8;
    default:
        return -1;
    }
}

// y-coordinate, 1-8
// function returns an index 0-7
const int Compass::rank_yindex(int sq)
{
    return sq >> 3;
}

// x-coordinate, a-h
// function returns an index 0-7
const int Compass::file_xindex(int sq)
{
    return sq % 8;
}

const int Compass::square_from_string(std::string str)
{
    int sq;
    switch (str[0])
    {
    case 'a':
        sq = 0;
        break;
    case 'b':
        sq = 1;
        break;
    case 'c':
        sq = 2;
        break;
    case 'd':
        sq = 3;
        break;
    case 'e':
        sq = 4;
        break;
    case 'f':
        sq = 5;
        break;
    case 'g':
        sq = 6;
        break;
    case 'h':
        sq = 7;
        break;
    }
    switch (str[1])
    {
    case '1':
        break;
    case '2':
        sq |= 1 << 3;
        break;
    case '3':
        sq |= 2 << 3;
        break;
    case '4':
        sq |= 3 << 3;
        break;
    case '5':
        sq |= 4 << 3;
        break;
    case '6':
        sq |= 5 << 3;
        break;
    case '7':
        sq |= 6 << 3;
        break;
    case '8':
        sq |= 7 << 3;
        break;
    }
    return sq;
}

const std::string Compass::string_from_square(int sq)
{
    std::string str = "";
    switch (file_xindex(sq))
    {
    case 0:
        str += 'a';
        break;
    case 1:
        str += 'b';
        break;
    case 2:
        str += 'c';
        break;
    case 3:
        str += 'd';
        break;
    case 4:
        str += 'e';
        break;
    case 5:
        str += 'f';
        break;
    case 6:
        str += 'g';
        break;
    case 7:
        str += 'h';
        break;
    }
    switch (rank_yindex(sq))
    {
    case 0:
        str += '1';
        break;
    case 1:
        str += '2';
        break;
    case 2:
        str += '3';
        break;
    case 3:
        str += '4';
        break;
    case 4:
        str += '5';
        break;
    case 5:
        str += '6';
        break;
    case 6:
        str += '7';
        break;
    case 7:
        str += '8';
        break;
    }
    return str;
}
