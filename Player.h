#ifndef PLAYER_H
#define PLAYER_H

#include "Chess.h"

class Player
{
public:
    Move get_move(Chess* ch);
    Move get_book_move(Chess* ch);
    float eval_position(Chess* ch);
private:
};

#endif
