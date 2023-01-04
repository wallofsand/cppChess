#include "Chess.h"

Chess::Chess()
{
    aci = 0;
    ply_counter = 0;
    // bl:QuKi wh:QuKi
    castle_rights = 0b1111;
    build_bitboards();
    std::vector<Move> history;
}

/*
 * Set up the starting position
 */
void Chess::build_bitboards()
{
    bb_white   = 0b1111111111111111;
    bb_black   = bb_white      << (8 * 6);
    bb_pawns   = 0b11111111ull << (8 * 6) | (0b11111111 << 8);
    bb_knights = 0b01000010ull << (8 * 7) | 0b01000010;
    bb_bishops = 0b00100100ull << (8 * 7) | 0b00100100;
    bb_rooks   = 0b10000001ull << (8 * 7) | 0b10000001;
    bb_queens  = 0b00001000ull << (8 * 7) | 0b00001000;
    bb_kings   = 0b00010000ull << (8 * 7) | 0b00010000;
    bb_occ     = bb_white                 | bb_black;
}

void Chess::make_move(Move mv)
{
    bool iscastle = false;
    ep_square = 0;
    int start = mv.start();
    int end = mv.end();
    for (int i = ch_cst::PAWN; i <= ch_cst::KING; i++)
    {
        // remove captured pieces
        *bb_by_piece[i] &= ~(1ull << end);
        *bb_by_color[1 - aci] &= ~(1ull << end);
        if (!Bitboard::contains_square(*bb_by_piece[i], start))
            continue;
        // remove the old piece
        *bb_by_piece[i] ^= 1ull << start;
        *bb_by_color[aci] ^= 1ull << start;
        // place the new piece
        *bb_by_piece[i] |= 1ull << end;
        *bb_by_color[aci] ^= 1ull << end;
        // update ep square
        if (i == ch_cst::PAWN && (start - end) % 16 == 0)
        {
            ep_square = start + directions::PAWN_DIR[aci];
        }
        // update castle rights
        if (i == ch_cst::KING)
        {
            if (start - end == 2 || start - end == -2)
                iscastle = true;
            castle_rights &= 3 << (2 * (1 - aci));
        }
        else if (i == ch_cst::ROOK)
            if (Compass::rank_yindex(start) == 0) // queenside rook moved
                castle_rights &= ~(2 << 2 * aci);
            else if (Compass::rank_yindex(start) == 7) // kingside rook moved
                castle_rights &= ~(1 << 2 * aci);
    }
    if (iscastle) // handle castles
    {
        if (start > end) // kingside castle
        {
            *bb_by_piece[ch_cst::ROOK] ^= 1ull << (start + 3);
            *bb_by_piece[ch_cst::ROOK] ^= 1ull << (end - 1);
        }
        else if (start < end) // queenside castle
        {
            *bb_by_piece[ch_cst::ROOK] ^= 1ull << (start - 4);
            *bb_by_piece[ch_cst::ROOK] ^= 1ull << (end + 1);
        }
    }
    ply_counter++;
    aci = ~aci & 1;
    history.push_back(mv);
}

void Chess::unmake_move(int undos)
{
    std::vector<Move> temp;
    for (Move m : history)
        temp.push_back(m);
    history.clear();
    aci = 0;
    ply_counter = 0;
    // bl:QuKi wh:QuKi
    castle_rights = 0b1111;
    build_bitboards();
    for (int i = 0; i < temp.size() - undos; i++)
    {
        make_move(temp[i]);
    }
}
