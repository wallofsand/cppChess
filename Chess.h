#ifndef CHESS_H
#define CHESS_H

#include "Move.h"
#include "Bitboard.h"
#include "Compass.h"
#include "TTable.h"
#include <string>
#include <vector>

class Chess
{
public:
    Chess();
    Chess(Chess& ch);
    int aci;
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
    int ply_counter;
    std::vector<Move> history;
    const int piece_at(int sq);
    const int color_at(int sq);
    // bl:QuKi wh:QuKi
    int castle_rights;
    void make_move(Move mv, bool test = false);
    void unmake_move(int undos);
    const void print_board(bool fmt = false);
    void build_bitboards();
    void hash();
    U64 zhash;
private:
    int size;
};

#endif
