#ifndef PLAYER_H
#define PLAYER_H

#include "Chess.h"
#include "PieceLocationTables.h"
#include "SearchLogger.h"
#include "MoveGenerator.h"

class Player
{
public:
    Move get_move(Chess& ch, int depth);
    Move get_book_move(Chess& ch);
    float eval(Chess& ch, int mate_offset = 0);
    float nega_max(Chess& ch, int depth, U64& nodes, float alpha = -99.99, float beta = 99.99);
    float quiescence_search(Chess& ch, int depth, U64& nodes, float alpha = -99.99, float beta = 99.99);
    float iterative_search(Chess& ch, int depth);
    std::vector<Move> order_moves_by_piece(Chess& ch, std::vector<Move>);
private:
    const float var_middlegame_weight = 32;
    const float var_mobility_weight = 0.01f;
    const float var_piece_value[7] = { 0, 1, 2.8f, 3, 5, 9.7f, 99.99f };
};

#endif
