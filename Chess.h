#ifndef CHESS_H
#define CHESS_H

#include <string>
#include <vector>
#include "Bitboard.h"
#include "Compass.h"
#include "Move.h"
#include "TTable.h"
#include "ChessStack.h"

namespace ch_cst
{
    const std::string START_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
}

class Chess
{
public:
    Chess();
    Chess(const Chess& ch);
    Chess(std::string fen);

    // Stack to store previous gamestates
    static ch_stk::ChessStack<Chess> stack;
    static inline Chess* state() { return stack.top->pos; }

    // bl:QuKi wh:QuKi
    int castle_rights;
    bool black_to_move;
    unsigned int halfmoves;
    unsigned int fullmoves;
    int ep_square;
    U64 bb_white{0};
    U64 bb_black{0};
    U64 bb_pawns{0};
    U64 bb_knights{0};
    U64 bb_bishops{0};
    U64 bb_rooks{0};
    U64 bb_queens{0};
    U64 bb_kings{0};
    U64 bb_occ{0};
    U64* bb_piece[7] = { nullptr, &bb_pawns, &bb_knights, &bb_bishops, &bb_rooks, &bb_queens, &bb_kings };
    U64* bb_color[2] = { &bb_white, &bb_black };
    U64 zhash;

    std::string fen() const;
    int piece_at(int sq) const;
    bool black_at(int sq) const;
    U64 hash() const;
    void print_board(bool fmt = false) const;
    int repetitions() const;

    // make_ and unmake_ methods
    static void push_move(const move mv, bool test = false);
    void make_move(move mv, bool test = false);
    static void unmake_move(uint32_t undos);
private:
    void build_bitboards();
};

#endif
