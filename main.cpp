#include "Chess.h"
#include "MoveGenerator.h"
#include <iostream>

char piece_char(int piece);

int main()
{
    Chess chess;
    std::string str = "";
    for (int sq = 0; sq < 64; sq++)
    {
        for (int color = 0; color < 2; color++)
        {
            for (int piece = constants::PAWN; piece <= constants::KING; piece++)
            {
                if (Bitboard::contains_square(*chess.piece_bbs[piece] & *chess.color_bbs[color], sq))
                {
                    str += piece_char(piece | color << 3);
                }
            }
        }
        if (str.length() > sq)
            continue;
        str += '.';
    }
    Bitboard::print_binary_string(str);
    return 0;
}

char piece_char(int piece)
{
    switch (piece)
    {
    case 1:
        return 'P';
    case 9:
        return 'p';
    case 2:
        return 'N';
    case 10:
        return 'n';
    case 3:
        return 'B';
    case 11:
        return 'b';
    case 4:
        return 'R';
    case 12:
        return 'r';
    case 5:
        return 'Q';
    case 13:
        return 'q';
    case 6:
        return 'K';
    case 14:
        return 'k';
    default:
        return '.';
    }
}
