#ifndef PLAYER_H
#define PLAYER_H

#include "Chess.h"
#include "TTable.h"
#include "PieceLocationTables.h"
#include "SearchLogger.h"
#include "MoveGenerator.h"

const int MOB_CONST = 4;

class Player {
public:
    Player(float mob_percent) : var_mobility_weight(MOB_CONST * mob_percent) {}
    move iterative_search(int depth, U64& nodes, bool test);
    move get_book_move(bool test);
    float nega_max(int depth, U64& nodes, float alpha = -99.99, float beta = 99.99, bool test = false);
    float quiescence_search(int depth, U64& nodes, float alpha = -99.99, float beta = 99.99, bool test = false);
    float eval(int mate_offset, bool test = false) const;
    float eval_position(float middlegame_weight) const;
    float eval_piece(float middlegame_weight, int piece, bool is_black) const;
    float king_safety(bool is_black) const;
    void order_moves_by_piece(move moves[MAXMOVES]) const;
private:
    float var_endgame_weight = 32.0f;
    float var_mobility_weight;
    int var_piece_value[7] = { 0, 100, 280, 300, 500, 970, 9999 };
};

#endif
