#ifndef CHESS_H
#define CHESS_H

#include <string>
#include <vector>
#include "Bitboard.h"
#include "Compass.h"
#include "Move.h"
#include "TTable.h"

class Chess
{
public:
    Chess();
    Chess(const Chess& ch);
    bool black_to_move;
    // bl:QuKi wh:QuKi
    int castle_rights;
    int ep_square = -1;
    U64 bb_white;
    U64 bb_black;
    U64 bb_pawns;
    U64 bb_knights;
    U64 bb_bishops;
    U64 bb_rooks;
    U64 bb_queens;
    U64 bb_kings;
    U64 bb_occ;
    U64* bb_piece[7] = { nullptr, &bb_pawns, &bb_knights, &bb_bishops, &bb_rooks, &bb_queens, &bb_kings };
    U64* bb_color[2] = { &bb_white, &bb_black };
    U64 zhash = 0ull;
    std::vector<move> history;
    std::vector<U64> prev_hash;
    int piece_at(int sq) const;
    int color_at(int sq) const;
    U64 hash() const;
    void print_board(bool fmt = false) const;
    void make_move(move mv, bool test = false);
    void unmake_move(int undos);
    int repetitions() const;
    void build_bitboards();
};

#endif
