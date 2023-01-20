#include "Compass.h"

U64 Compass::knight_attacks[64];
U64 Compass::king_attacks[64];
uint8_t Compass::first_rank_attacks_64x8[64*8];
int8_t Compass::edge_distance_64x8[64][8];

Compass::Compass()
{
    // init stuff
    compute_edge_distances();
    compute_knight_attacks();
    compute_rank_attacks();
    compute_king_attacks();
}

// Method to return a bitboard of the ray which
// passes through square sq in direction DIRS[dir_index]
const U64 Compass::build_ray(int sq, int dir_index)
{
    U64 ray = 0;
    for (int step = 1; step <= Compass::edge_distance_64x8[sq][dir_index]; step++)
    {
        ray |= 1ull << (sq + step * directions::DIRS[dir_index]);
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
        int dir = directions::DIRS[i];
        for (int step = 1; step <= edge_distance_64x8[sq0][i]; step++)
            if (sq0 + step * directions::DIRS[i] == sq1)
            {
                U64 ray = 0;
                int sq = sq0;
                while(sq != sq1)
                {
                    ray |= 1ull << sq;
                    sq += directions::DIRS[i];
                }
                return ray;
            }
    }
    return 0;
}

// Method to return a bitboard of the ray from squares
// start (exclusive) through square end to board edge
// retuns zero if no such ray exists
// an optional occupancy array can be passed to stop the ray
const U64 Compass::ray_square(int start, int end, U64 occ)
{
    U64 ray = BB::nort_attacks(1ull << start, ~occ);
    if (BB::contains_square(ray, end))
        return ray;
    ray = BB::NoEa_attacks(1ull << start, ~occ);
    if (BB::contains_square(ray, end))
        return ray;
    ray = BB::east_attacks(1ull << start, ~occ);
    if (BB::contains_square(ray, end))
        return ray;
    ray = BB::SoEa_attacks(1ull << start, ~occ);
    if (BB::contains_square(ray, end))
        return ray;
    ray = BB::sout_attacks(1ull << start, ~occ);
    if (BB::contains_square(ray, end))
        return ray;
    ray = BB::SoWe_attacks(1ull << start, ~occ);
    if (BB::contains_square(ray, end))
        return ray;
    ray = BB::west_attacks(1ull << start, ~occ);
    if (BB::contains_square(ray, end))
        return ray;
    ray = BB::NoWe_attacks(1ull << start, ~occ);
    if (BB::contains_square(ray, end))
        return ray;
    return 0ull;
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
        edge_distance_64x8[sq][0] = nstep;
        edge_distance_64x8[sq][1] = estep;
        edge_distance_64x8[sq][2] = sstep;
        edge_distance_64x8[sq][3] = wstep;
        edge_distance_64x8[sq][4] = nstep < estep ? nstep : estep;
        edge_distance_64x8[sq][5] = nstep < wstep ? nstep : wstep;
        edge_distance_64x8[sq][6] = sstep < estep ? sstep : estep;
        edge_distance_64x8[sq][7] = sstep < wstep ? sstep : wstep;
    }
}

void Compass::compute_knight_attacks()
{
    using namespace directions;
    for (int sq = 0; sq < 64; sq++)
    {
        // NORTH, EAST, SOUTH, WEST
        // NNE, NEE, SEE, SSE, SSW, SWW, NWW, NNW;
        Compass::knight_attacks[sq] = 0b0;
        if (edge_distance_64x8[sq][0] > 0)
        {
            if (edge_distance_64x8[sq][0] > 1 && edge_distance_64x8[sq][1] > 0)
                Compass::knight_attacks[sq] |= 1ull << (sq + NNE);
            if (edge_distance_64x8[sq][1] > 1)
                Compass::knight_attacks[sq] |= 1ull << (sq + NEE);
            if (edge_distance_64x8[sq][0] > 1 && edge_distance_64x8[sq][3] > 0)
                Compass::knight_attacks[sq] |= 1ull << (sq + NNW);
            if (edge_distance_64x8[sq][3] > 1)
                Compass::knight_attacks[sq] |= 1ull << (sq + NWW);
        }
        if (edge_distance_64x8[sq][2] > 0)
        {
            if (edge_distance_64x8[sq][2] > 1 && edge_distance_64x8[sq][1] > 0)
                Compass::knight_attacks[sq] |= 1ull << (sq + SSE);
            if (edge_distance_64x8[sq][1] > 1)
                Compass::knight_attacks[sq] |= 1ull << (sq + SEE);
            if (edge_distance_64x8[sq][2] > 1 && edge_distance_64x8[sq][3] > 0)
                Compass::knight_attacks[sq] |= 1ull << (sq + SSW);
            if (edge_distance_64x8[sq][3] > 1)
                Compass::knight_attacks[sq] |= 1ull << (sq + SWW);
        }
    }
}

const U64 Compass::rank_attacks(U64 occ, int sq)
{
    int rook_file = file_xindex(sq);
    int rank_times8 = sq & 56; // rank * 8
    int rank_occ_times2 = (occ >> rank_times8) & 2 * 63;
    // return 8 * rank_occ + file shifted to the right rank
    return first_rank_attacks_64x8[4*rank_occ_times2+rook_file] << rank_times8;
}

void Compass::compute_rank_attacks()
{
    for (U64 occ = 0; occ < 64; occ++) for (int rksq = 0; rksq < 8; rksq++)
    {
        U64 rook = 1ull << rksq;
        first_rank_attacks_64x8[8*occ+rksq] = BB::east_attacks(rook, ~occ) & 255;
        first_rank_attacks_64x8[8*occ+rksq] |= BB::west_attacks(rook, ~occ) & 255;
        first_rank_attacks_64x8[8*occ+rksq] &= ~rook;
    }
}

void Compass::compute_king_attacks()
{
    for (int king_sq = 0; king_sq < 64; king_sq++)
    {
        king_attacks[king_sq] = 0ull;
        for (int dir_idx = get_dir_start_index(ch_cst::KING); dir_idx < get_dir_end_index(ch_cst::KING); dir_idx++)
        {
            int dir = directions::DIRS[dir_idx];
            if (edge_distance_64x8[king_sq][dir_idx] == 0)
                continue;
            king_attacks[king_sq] |= 1ull << (king_sq + dir);
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
