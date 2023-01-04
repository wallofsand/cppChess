#include "Chess.h"

using namespace ch_cst;

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
    ep_square = 0;
    int start = mv.start();
    int end = mv.end();
    bool castle = false;

    // what type of piece is moving?
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
            ep_square = start + directions::PAWN_DIR[aci];

        // update castle rights
        if (i == ch_cst::KING)
        {
            // handle castles
            // kingside castle
            if (end - start == -2)
            {
                *bb_by_piece[ch_cst::ROOK] ^= 1ull << (start + 3);
                *bb_by_piece[ch_cst::ROOK] ^= 1ull << (end - 1);
            }
            // queenside castle
            else if (end - start == 2)
            {
                *bb_by_piece[ch_cst::ROOK] ^= 1ull << (start - 4);
                *bb_by_piece[ch_cst::ROOK] ^= 1ull << (end + 1);
            }
            castle_rights &= 3 << (2 * (1 - aci));
        }
        else if (i == ch_cst::ROOK)
            // queenside rook moved
            if (Compass::rank_yindex(start) == 0)
                castle_rights &= ~(2 << 2 * aci);
            // kingside rook moved
            else if (Compass::rank_yindex(start) == 7)
                castle_rights &= ~(1 << 2 * aci);
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
    castle_rights = 0b1111;
    build_bitboards();
    for (int i = 0; i < temp.size() - undos; i++)
    {
        make_move(temp[i]);
    }
}

std::string Chess::move_fen(Move mv)
{
    std::string fen = "";
    int start = mv.start(), end = mv.end();
    int piece_moved, capture;

    for (int type = PAWN; type <= KING; type++)
    {
        if (!Bitboard::contains_square(*bb_by_piece[type], start))
            continue;

        fen += piece_char[aci << 3 | type];

        // captures
        if (Bitboard::contains_square(bb_occ, end))
            fen += "x";

        fen += square_string[end];

        // castling
        if (type == KING && (start - end == 2 || start - end == -2))
        {
            // short castle
            fen = "O-O";
            // long castle
            if (start - end == 2)
                fen +="-O";
        }

        // promotions
        if (mv.promote())
            fen += piece_char[mv.promote()];
    }

    return fen;
}
