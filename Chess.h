#ifndef CHESS_H
#define CHESS_H

#include "Bitboard.h"
#include "Compass.h"
#include "Move.h"
#include "TTable.h"
#include <string>
#include <vector>

namespace Chess
{
    struct State
    {
        State();
        State(const State& _ch);
        bool black_to_move;
        // bl:QuKi wh:QuKi
        int castle_rights;
        int ep_square;
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
        U64 zhash;
        std::vector<move> history;
    };
    const int piece_at(const State& ch, int sq);
    const int color_at(const State& ch, int sq);
    void make_move(State& ch, move mv, bool test = false);
    void unmake_move(State& ch, int undos);
    const void print_board(const State& ch, bool fmt = false);
    const int repetitions(const State& ch);
    void build_bitboards(State& ch);
    const U64 hash(const State& ch);
};

#endif
