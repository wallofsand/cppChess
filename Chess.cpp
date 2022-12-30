#include "Chess.h"

Chess::Chess()
{
    aci = 0;
    ply_counter = 0;
    // wh:QuKi bl:QuKi
    castle_rights = 0b1111;
    build_bitboards();
    std::vector<Move> history;
}

Chess::~Chess() {}

void Chess::build_bitboards()
{
    bb_white = 0b1111111111111111;
    bb_black = bb_white << (8 * 6);
    bb_pawns = 0b11111111ull << (8 * 6) | (0b11111111 << 8);
    bb_knights = 0b01000010ull << (8 * 7) | 0b01000010;
    bb_bishops = 0b00100100ull << (8 * 7) | 0b00100100;
    bb_rooks = 0b10000001ull << (8 * 7) | 0b10000001;
    bb_queens = 0b00010000ull << (8 * 7) | 0b00010000;
    bb_kings = 0b00001000ull << (8 * 7) | 0b00001000;
    bb_occ = bb_white | bb_black;
}

void Chess::make_move(Move mv, bool record)
{
    bool iscastle = false;
    for (int i = ch_cst::PAWN; i <= ch_cst::KING; i++)
    {
        // remove captured pieces
        *bb_by_piece[i] &= ~(1ull << mv.end);
        *bb_by_color[1 - aci] &= ~(1ull << mv.end);
        if (Bitboard::contains_square(*bb_by_piece[i], mv.start))
        {
            // remove the old piece
            *bb_by_piece[i] ^= 1ull << mv.start;
            *bb_by_color[aci] ^= 1ull << mv.start;
            // place the new piece
            *bb_by_piece[i] |= 1ull << mv.end;
            *bb_by_color[aci] ^= 1ull << mv.end;
            // update castle rights
            if (i == ch_cst::KING)
            {
                if (mv.start - mv.end == 2 || mv.start - mv.end == -2)
                    iscastle = true;
                castle_rights &= 0b11 << (2 * aci);
            }
            else if (i == ch_cst::ROOK)
                if (Compass::rank_yindex(mv.start) == 0) // queenside rook moved
                    castle_rights &= ~(2 << 2 * (1 - aci));
                else if (Compass::rank_yindex(mv.start) == 7) // kingside rook moved
                    castle_rights &= ~(1 << 2 * (1 - aci));
        }
    }
    if (iscastle) // handle castles
    {
        if (mv.start > mv.end) // kingside castle
        {
            *bb_by_piece[ch_cst::ROOK] ^= 1ull << (mv.start + 3);
            *bb_by_piece[ch_cst::ROOK] ^= 1ull << (mv.end - 1);
        }
        else if (mv.start < mv.end) // queenside castle
        {
            *bb_by_piece[ch_cst::ROOK] ^= 1ull << (mv.start - 4);
            *bb_by_piece[ch_cst::ROOK] ^= 1ull << (mv.end + 1);
        }
    }
    ply_counter++;
    aci = ~aci & 1;
    if (record)
        history.push_back(mv);
}

void Chess::unmake_move(int undos)
{
    build_bitboards();
    ep_square = 0;
    for (int i = 0; i < history.size() - undos; i++)
    {
        make_move(history[i], false);
    }
    for (int i = 0; i < undos; i++)
        history.pop_back();
}
