#ifndef MOVEGENERATOR_H
#define MOVEGENERATOR_H

#include "Chess.h"
#include "Compass.h"
#include "Timer.h"
#include "SearchLogger.h"
#include <algorithm>
#include <vector>

class MoveGenerator
{
public:
    inline MoveGenerator(Chess& pos) : ch(pos) { init(false); }
    Chess& ch;
    bool is_game_over(bool test);
    int find_king(int color);
    void gen_moves(move* moves, bool test = false);
    void checks_exist(bool test);
    bool in_check = false;
    bool in_double_check = false;
    U64 gen_op_attack_mask(bool test);
    U64 pinned_pieces;
    void init(bool test);
    static std::string move_san(Chess ch, move mv);
private:
    void gen_pawn_moves(move* pawn_moves, bool test);
    void gen_knight_piece_moves(move* knight_moves, int sq, bool test);
    void gen_bishop_piece_moves(move* bishop_moves, int sq, bool test);
    void gen_rook_piece_moves(move* rook_moves, int sq, bool test);
    void gen_king_piece_moves(move* king_moves, bool test);
    U64 find_pins(bool test);
    void check_method();
    U64 check_ray;
    U64 op_attack_mask;
};

#endif
