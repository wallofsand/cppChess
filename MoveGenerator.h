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
    const bool is_game_over();
    const int find_king(int color);
    const std::vector<Move> gen_moves();
    static U64 perft_root(MoveGenerator& perft_gen, Chess& ch, int depth, bool initial_pos, int log_depth);
private:
    static U64 perft(MoveGenerator& perft_gen, Chess& ch, int depth, SearchLogger& perft_log);
    const std::vector<Move> gen_pawn_moves();
    const std::vector<Move> gen_knight_piece_moves(int sq);
    const std::vector<Move> gen_bishop_piece_moves(int sq);
    const std::vector<Move> gen_rook_piece_moves(int sq);
    const std::vector<Move> gen_king_piece_moves(int sq);
    void find_pins();
    U64 gen_op_attack_mask();
    void check_exists();
    void check_method();
    Chess chess;
    U64 check_ray;
    U64 pinned_pieces;
    U64 pin_ray_moves;
    U64 op_attack_mask;
};

#endif
