#ifndef MOVEGENERATOR_H
#define MOVEGENERATOR_H

#include "Chess.h"
#include "Compass.h"
#include <vector>

class MoveGenerator
{
public:
    MoveGenerator(Chess* ch);
    ~MoveGenerator();
    Chess* chess;
    std::vector<Move> gen_moves();
    std::vector<Move> gen_pawn_piece_moves(int sq);
    std::vector<Move> gen_knight_piece_moves(int sq);
    std::vector<Move> gen_sliding_piece_moves(int sq, int piece);
    std::vector<Move> gen_king_piece_moves(int sq);
private:
};

#endif
