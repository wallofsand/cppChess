#ifndef MOVEGENERATOR_H
#define MOVEGENERATOR_H

#include "Chess.h"
#include "Compass.h"
#include "Timer.h"
#include <algorithm>
#include <vector>

class MoveGenerator
{
public:
    Chess chess;
    void set_chess(Chess* ch);
    std::vector<Move> gen_moves();
    U64 perft_root(int depth);
    U64 perft(int depth);
private:
    bool in_check;
    bool in_double_check;
    int check_square;
    U64 check_ray;
    U64 pinned_pieces;
    U64 pin_ray_moves;
    U64 op_attack_mask;
    void find_pins();
    int find_king(int color);
    void gen_op_attack_mask();
    bool is_game_over();
    void check_exists();
    void check_method(int sq);
    std::vector<Move> gen_pawn_moves();
    std::vector<Move> gen_knight_piece_moves(int sq);
    std::vector<Move> gen_bishop_piece_moves(int sq);
    std::vector<Move> gen_rook_piece_moves(int sq);
    std::vector<Move> gen_king_piece_moves(int sq);
};

#endif
