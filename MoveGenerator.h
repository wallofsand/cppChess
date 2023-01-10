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
    MoveGenerator(Chess& ch);
    Chess& ch;
    bool is_game_over(bool test);
    int find_king(int color);
    std::vector<Move> gen_moves(bool test = false);
    std::string MoveGenerator::move_san(Move& mv);
    void checks_exist(bool test);
    void init(bool test);
    bool in_check = false;
    bool in_double_check = false;
    U64 gen_op_attack_mask(bool test);
private:
    std::vector<Move> gen_pawn_moves(bool test);
    std::vector<Move> gen_knight_piece_moves(int sq, bool test);
    std::vector<Move> gen_bishop_piece_moves(int sq, bool test);
    std::vector<Move> gen_rook_piece_moves(int sq, bool test);
    std::vector<Move> gen_king_piece_moves(int sq, bool test);
    void find_pins(bool test);
    void check_method();
    U64 check_ray;
    U64 pinned_pieces;
    U64 op_attack_mask;
};

#endif
