#include "PieceLocationTables.h"

const int* PieceLocationTables::middlegame_piece_tables[7] = { nullptr, PieceLocationTables::pawns, PieceLocationTables::knights, PieceLocationTables::bishops, PieceLocationTables::rooks, PieceLocationTables::queens, PieceLocationTables::kings_middle};
const int* PieceLocationTables::endgame_piece_tables[7] = { nullptr, PieceLocationTables::pawns, PieceLocationTables::knights, PieceLocationTables::bishops, PieceLocationTables::rooks, PieceLocationTables::queens, PieceLocationTables::kings_end};

// all tables are read from black's perspective
// when white reads a table, we need to flip it along the Y axis
const int PieceLocationTables::read(const int* table, int sq, bool is_black)
{
    if (!is_black)
    {
        int file = Compass::file_xindex(sq);
        int rank = 7 - Compass::rank_yindex(sq);
        sq = 8 * rank + file;
    }
    return table[sq];
}

// method to interpolate normal and endgame tables
// middlegameWeight is the number of pieces / 32 - a value between 0.0 and 1.0
const float PieceLocationTables::complex_read(int type, int sq, float middlegame_weight, bool is_black)
{
    if (type < 1 || type > 6)
        return 0;
    float opening = read(middlegame_piece_tables[type], sq, is_black) * middlegame_weight;
    float endgame = read(endgame_piece_tables[type], sq, is_black) * (1 - middlegame_weight);
    return opening + endgame;
}
