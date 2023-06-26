#ifndef CHESS_H
#define CHESS_H

#include <string>
#include <vector>
#include "Bitboard.h"
#include "Compass.h"
#include "Move.h"
#include "TTable.h"
#include "ChessStack.h"

namespace ch_cst {
    const std::string TEST_FEN = "r1bqk2r/ppp2ppp/2np4/2P1p3/4Pn2/3B1N2/PBPP1P1P/R2Q2KR b kq - 11 14";
    const std::string LONG_FEN = "r2qkbnr/p4p2/1ppp2Pp/4p3/4P1Q1/P1NP2P1/1P3P2/n1BK1BNR w kq - 0 12";
    const std::string START_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
} // namespace ch_cst

class Chess {
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
    U64 bb_white = 0;
    U64 bb_black = 0;
    U64 bb_pawns = 0;
    U64 bb_knights = 0;
    U64 bb_bishops = 0;
    U64 bb_rooks = 0;
    U64 bb_queens = 0;
    U64 bb_kings = 0;
    U64 bb_occ = 0;
    U64* bb_piece[7] = { nullptr, &bb_pawns, &bb_knights, &bb_bishops, &bb_rooks, &bb_queens, &bb_kings };
    U64* bb_color[2] = { &bb_white, &bb_black };
    U64 zhash;

    std::string fen() const;
    int find_king(bool is_black) const;
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
