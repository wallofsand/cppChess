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
    MoveGenerator(Chess &ch);
    void set_chess(Chess &ch);
    void init();
    bool in_check;
    bool in_double_check;
    bool is_game_over();
    int find_king(int color);
    std::vector<Move> gen_moves();
    Chess chess;
private:
    std::vector<Move> gen_pawn_moves();
    std::vector<Move> gen_knight_piece_moves(int sq);
    std::vector<Move> gen_bishop_piece_moves(int sq);
    std::vector<Move> gen_rook_piece_moves(int sq);
    std::vector<Move> gen_king_piece_moves(int sq);
    void find_pins();
    void gen_op_attack_mask();
    void check_exists();
    void check_method();
    U64 check_ray;
    U64 pinned_pieces;
    U64 pin_ray_moves;
    U64 op_attack_mask;
};

#endif
