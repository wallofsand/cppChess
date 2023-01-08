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
    MoveGenerator();
    bool is_game_over(Chess& chess);
    int find_king(Chess& chess, int color);
    std::vector<Move> gen_moves(Chess& chess);
    std::string MoveGenerator::move_san(Chess& chess, Move& mv);
    void check_exists(Chess& chess);
// private:
    std::vector<Move> gen_pawn_moves(Chess& chess);
    std::vector<Move> gen_knight_piece_moves(Chess& chess, int sq);
    std::vector<Move> gen_bishop_piece_moves(Chess& chess, int sq);
    std::vector<Move> gen_rook_piece_moves(Chess& chess, int sq);
    std::vector<Move> gen_king_piece_moves(Chess& chess, int sq);
    void find_pins(Chess& chess);
    U64 gen_op_attack_mask(Chess& chess);
    void check_method(Chess& chess);
    U64 check_ray;
    U64 pinned_pieces;
    U64 pin_ray_moves;
    U64 op_attack_mask;
};

#endif
