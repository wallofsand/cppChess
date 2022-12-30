#ifndef CHESS_H
#define CHESS_H

#include "Move.h"
#include "Bitboard.h"
#include "Compass.h"
#include <vector>

namespace ch_cst
{
    const U64 notafile = 0xfefefefefefefefe;
    const U64 nothfile = 0x7f7f7f7f7f7f7f7f;
    const int PAWN = 1;
    const int KNIGHT = 2;
    const int BISHOP = 3;
    const int ROOK = 4;
    const int QUEEN = 5;
    const int KING = 6;
    const int WHITE_INDEX = 0;
    const int BLACK_INDEX = 1;
}

class Chess
{
public:
    Chess();
    ~Chess();
    int ep_square;
    bool game_over = false;
    U64 bb_white;
    U64 bb_black;
    U64 bb_pawns;
    U64 bb_knights;
    U64 bb_bishops;
    U64 bb_rooks;
    U64 bb_queens;
    U64 bb_kings;
    U64 bb_occ;
    U64* bb_by_piece[7] = { nullptr, &bb_pawns, &bb_knights, &bb_bishops, &bb_rooks, &bb_queens, &bb_kings };
    U64* bb_by_color[2] = { &bb_white, &bb_black };
    int aci;
    int ply_counter;
    std::vector<Move> history;
    int castle_rights;
    void make_move(Move mv, bool record = true);
    void unmake_move(int undos);
private:
    void build_bitboards();
};

#endif
