#ifndef PLAYER_H
#define PLAYER_H

#include "Chess.h"
#include "TTable.h"
#include "PieceLocationTables.h"
#include "SearchLogger.h"
#include "MoveGenerator.h"
#include <stdio.h>

class Player
{
public:
    Player(float delta) : var_mobility_weight(0.1f + delta) {};
    Move get_move(Chess& ch, SearchLogger& search_log, int8_t depth, U64& nodes, bool test);
    Move iterative_search(Chess& ch, int8_t depth, U64& nodes, bool test);
    Move get_book_move(Chess& ch, SearchLogger& search_log, bool test);
    float nega_max(Chess& ch, SearchLogger& search_log, int8_t depth, U64& nodes, float alpha = -99.99, float beta = 99.99, bool test = false);
    float quiescence_search(Chess& ch, SearchLogger& search_log, int8_t depth, U64& nodes, float alpha = -99.99, float beta = 99.99, bool test = false);
    float eval(Chess& ch, std::vector<Move> moves, int8_t mate_offset = 0, bool test = false);
    float eval(Chess& ch, std::vector<Move> moves, SearchLogger& search_log, int8_t mate_offset = 0, bool test = false);
    std::vector<Move> order_moves_by_piece(Chess& ch, std::vector<Move>);
private:
    float var_endgame_weight = 32.0f;
    float var_mobility_weight = 0.1f;
    float var_piece_value[7] = { 0, 1, 2.8f, 3, 5, 9.7f, 99.99f };
};

#endif
