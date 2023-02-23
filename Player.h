#ifndef PLAYER_H
#define PLAYER_H

#include "Chess.h"
#include "TTable.h"
#include "PieceLocationTables.h"
#include "SearchLogger.h"
#include "MoveGenerator.h"

const int MOB_CONST = 5;

class Player
{
public:
    Player(float delta);
    move iterative_search(Chess& ch, int8_t depth, U64& nodes, bool test);
    move get_book_move(Chess& ch, bool test);
    float nega_max(Chess& ch, int8_t depth, U64& nodes, float alpha = -99.99, float beta = 99.99, bool test = false);
    float quiescence_search(Chess& ch, int8_t depth, U64& nodes, float alpha = -99.99, float beta = 99.99, bool test = false);
    float eval(Chess& ch, int8_t mate_offset, bool test = false);
    bool move_is_check(Chess ch, move mv);
    void order_moves_by_piece(Chess& ch, const move moves[120], move* ordered) const;
private:
    SearchLogger search_log;
    float var_endgame_weight = 32.0f;
    float var_mobility_weight;
    int var_piece_value[7] = { 0, 100, 280, 300, 500, 970, 9999 };
};

#endif
