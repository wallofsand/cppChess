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
    move* gen_moves(bool test = false);
    void checks_exist(bool test);
    bool in_check = false;
    bool in_double_check = false;
    U64 gen_op_attack_mask(bool test);
    U64 pinned_pieces;
    void init(bool test);
    static std::string move_san(Chess ch, move mv);
private:
    move* gen_pawn_moves(bool test);
    move* gen_knight_piece_moves(int sq, bool test);
    move* gen_bishop_piece_moves(int sq, bool test);
    move* gen_rook_piece_moves(int sq, bool test);
    move* gen_king_piece_moves(int sq, bool test);
    U64 find_pins(bool test);
    void check_method();
    U64 check_ray;
    U64 op_attack_mask;
};

#endif
