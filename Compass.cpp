#include "Compass.h"
#include "Chess.h"
#include <iostream>

ULL Compass::KNIGHT_ATTACKS[64];
ULL Compass::KING_ATTACKS[64];
uint8_t Compass::ROOK_ROWS[(8 << 6) | (7 << 3) | 6];
int Compass::edge_distance64x8[64][8];

Compass::Compass()
{
    // init stuff
    compute_edge_distances();
    compute_knight_attacks();
    compute_rook_attacks();
    compute_king_attacks();
}

void Compass::compute_edge_distances()
{
    for (int sq = 0; sq < 64; sq++)
    {
        int rank = rank_index(sq);
        int file = file_index(sq);
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
        Compass::KNIGHT_ATTACKS[sq] = 0b0;
        if (edge_distance64x8[sq][0] > 0)
        {
            if (edge_distance64x8[sq][0] > 1 && edge_distance64x8[sq][1] > 0)
                Compass::KNIGHT_ATTACKS[sq] |= 1ull << (sq + NNE);
            if (edge_distance64x8[sq][1] > 1)
                Compass::KNIGHT_ATTACKS[sq] |= 1ull << (sq + NEE);
            if (edge_distance64x8[sq][0] > 1 && edge_distance64x8[sq][3] > 0)
                Compass::KNIGHT_ATTACKS[sq] |= 1ull << (sq + NNW);
            if (edge_distance64x8[sq][3] > 1)
                Compass::KNIGHT_ATTACKS[sq] |= 1ull << (sq + NWW);
        }
        if (edge_distance64x8[sq][2] > 0)
        {
            if (edge_distance64x8[sq][2] > 1 && edge_distance64x8[sq][1] > 0)
                Compass::KNIGHT_ATTACKS[sq] |= 1ull << (sq + SSE);
            if (edge_distance64x8[sq][1] > 1)
                Compass::KNIGHT_ATTACKS[sq] |= 1ull << (sq + SEE);
            if (edge_distance64x8[sq][2] > 1 && edge_distance64x8[sq][3] > 0)
                Compass::KNIGHT_ATTACKS[sq] |= 1ull << (sq + SSW);
            if (edge_distance64x8[sq][3] > 1)
                Compass::KNIGHT_ATTACKS[sq] |= 1ull << (sq + SWW);
        }
    }
}

void Compass::compute_rook_attacks()
{
    for (int rksq = 0; rksq < 8; rksq++)
    {
        for (int r = 0; r < rksq; r++)
        {
            for (int l = rksq + 1; l < 8; l++)
            {
                // table is indexed by [(l << 6) | (rksq << 3) | r]
                // max size: [(8 << 6) | (7 << 3) | 6]
                int idx = (l << 6) | (rksq << 3) | r;
                // with a rook on rksq, and blocking pieces on r and l
                // flip the bits the rook can attack
                ROOK_ROWS[idx] = 0;
                ROOK_ROWS[idx] |= 1 << rksq;
                for (int i = 0; i <= r; i++)
                    ROOK_ROWS[idx] |= 1 << i;
                for (int i = l; i < 8; i++)
                    ROOK_ROWS[idx] |= 1 << i;
                ROOK_ROWS[idx] = ~ROOK_ROWS[idx];
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
}

void Compass::compute_king_attacks()
{
    for (int sq = 0; sq < 64; sq++)
    {
        KING_ATTACKS[sq] = 0b0;
        for (int dir_idx = get_dir_start_index(constants::KING); dir_idx < get_dir_end_index(constants::KING); dir_idx++)
        {
            int dir = directions[dir_idx];
            if (edge_distance64x8[sq][dir_idx] == 0)
                continue;
            KING_ATTACKS[sq] |= 1ull << (sq + dir);
        }
    }
}

int Compass::get_dir_start_index(int piece)
{
    switch (piece)
    {
    case constants::KNIGHT:
        return 8;
    case constants::BISHOP:
        return 4;
    case constants::ROOK:
    case constants::QUEEN:
    case constants::KING:
        return 0;
    default:
        return -1;
    }
}

int Compass::get_dir_end_index(int piece)
{
    switch (piece)
    {
    case constants::KNIGHT:
        return 16;
    case constants::ROOK:
        return 4;
    case constants::BISHOP:
    case constants::QUEEN:
    case constants::KING:
        return 8;
    default:
        return -1;
    }
}

uint8_t Compass::get_rook_row(int rksq, int lsq, int rsq)
{
    // table is indexed by [(l << 6) | (rksq << 3) | r]
    int rk_file = file_index(rksq);
    int l_file = file_index(lsq);
    int r_file = file_index(rsq);
    return ROOK_ROWS[(l_file << 6) | (rk_file << 3) | r_file];
}

// y-coordinate, 1-8
// function returns an index 0-7
int Compass::rank_index(int sq)
{
    return sq >> 3;
}

// x-coordinate, a-h
// function returns an index 0-7
int Compass::file_index(int sq)
{
    return sq % 8;
}