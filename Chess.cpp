#include "Chess.h"

Chess::Chess()
{
    active_color_index = 0;
    ply_counter = 0;
    // wh:QuKi bl:QuKi
    castle_rights = 0b1111;
    build_bitboards();
}

Chess::~Chess() {}

void Chess::build_bitboards()
{
    bbwhite = 0b1111111111111111;
    bbblack = bbwhite << (8 * 6);
    bbpawns = 0b11111111ull << (8 * 6) | (0b11111111 << 8);
    bbknights = 0b01000010ull << (8 * 7) | 0b01000010;
    bbbishops = 0b00100100ull << (8 * 7) | 0b00100100;
    bbrooks = 0b10000001ull << (8 * 7) | 0b10000001;
    bbqueens = 0b00010000ull << (8 * 7) | 0b00010000;
    bbkings = 0b00001000ull << (8 * 7) | 0b00001000;
    bbocc = bbwhite | bbblack;
}

void Chess::make_move(Move mv, bool record)
{
    for (int i = constants::PAWN; i <= constants::KING; i++)
    {
        // remove captured pieces
        *piece_bbs[i] &= ~(1ull << mv.end);
        *color_bbs[1 - active_color_index] &= ~(1ull << mv.end);
        if (Bitboard::contains_square(*piece_bbs[i], mv.start))
        {
            // remove the old piece
            *piece_bbs[i] ^= 1ull << mv.start;
            *color_bbs[active_color_index] ^= 1ull << mv.start;
            // place the new piece
            *piece_bbs[i] |= 1ull << mv.end;
            *color_bbs[active_color_index] ^= 1ull << mv.end;
            // update castle rights
            if (i == constants::KING)
                castle_rights &= 0b11 << (2 * active_color_index);
            else if (i == constants::ROOK)
                if (Compass::rank_index(mv.start) == 0) // queenside rook moved
                    castle_rights &= ~(2 << 2 * (1 - active_color_index));
                else if (Compass::rank_index(mv.start) == 7) // kingside rook moved
                    castle_rights &= ~(1 << 2 * (1 - active_color_index));
        }
    }
    if (mv.flag == 1) // handle castles
    {
        if (mv.start > mv.end) // kingside castle
        {
            *piece_bbs[constants::ROOK] ^= 1ull << (mv.start + 3);
            *piece_bbs[constants::ROOK] ^= 1ull << (mv.end - 1);
        }
        else if (mv.start < mv.end) // queenside castle
        {
            *piece_bbs[constants::ROOK] ^= 1ull << (mv.start - 4);
            *piece_bbs[constants::ROOK] ^= 1ull << (mv.end + 1);
        }
    }
    ply_counter++;
    active_color_index = ~active_color_index & 1;
    if (record)
        history.push_back(mv);
}
