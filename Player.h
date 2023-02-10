#ifndef PLAYER_H
#define PLAYER_H

#include "Chess.h"
#include "TTable.h"
#include "PieceLocationTables.h"
#include "SearchLogger.h"
#include "MoveGenerator.h"

class Player
{
public:
    Player(float delta);
    move get_move(Chess& ch, int depth, U64& nodes, bool test);
    move iterative_search(Chess ch, int depth, U64& nodes, bool test);
    move get_book_move(Chess& ch, bool test);
    float nega_max(Chess& ch, int depth, U64& nodes, float alpha = -99.99, float beta = 99.99, bool test = false);
    float quiescence_search(Chess& ch, int depth, U64& nodes, float alpha = -99.99, float beta = 99.99, bool test = false);
    float eval(Chess& ch, int mate_offset = 0, bool test = false);
    bool move_is_check(Chess& ch, move mv);
    std::vector<move> order_moves_by_piece(Chess& ch, std::vector<move>) const;
private:
    SearchLogger search_log;
    float var_endgame_weight = 32.0f;
    float var_mobility_weight;
    float var_piece_value[7] = { 0, 1, 2.8f, 3, 5, 9.7f, 99.99f };
};

#endif
