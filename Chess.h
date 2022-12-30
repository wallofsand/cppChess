#ifndef CHESS_H
#define CHESS_H

#include "Move.h"
#include "Bitboard.h"
#include "Compass.h"
#include <vector>

namespace constants
{
    const int PAWN = 1;
    const int KNIGHT = 2;
    const int BISHOP = 3;
    const int ROOK = 4;
    const int QUEEN = 5;
    const int KING = 6;
    const int WHITE_INDEX = 0;
    const int BLACK_INDEX = 1;
}

class Chess
{
public:
    Chess();
    ~Chess();
    uint8_t ep_square;
    bool gameover = false;
    ULL bbwhite;
    ULL bbblack;
    ULL bbpawns;
    ULL bbknights;
    ULL bbbishops;
    ULL bbrooks;
    ULL bbqueens;
    ULL bbkings;
    ULL bbocc;
    ULL* piece_bbs[7] = { nullptr, &bbpawns, &bbknights, &bbbishops, &bbrooks, &bbqueens, &bbkings };
    ULL* color_bbs[2] = { &bbwhite, &bbblack };
    int active_color_index;
    int ply_counter;
    std::vector<Move> history;
    int castle_rights;
    void make_move(Move mv, bool record = true);
    void unmake_move(int undos);
private:
    void build_bitboards();
};

#endif
