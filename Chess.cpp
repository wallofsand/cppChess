#include "Chess.h"

Chess::Chess()
{
    aci = 0;
    ply_counter = 0;
    castle_rights = 0b1111;
    in_check = false;
    in_double_check = false;
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

int Chess::piece_at(int sq)
{
    for (int piece = ch_cst::PAWN; piece <= ch_cst::KING; piece++)
    {
        if (Bitboard::contains_square(*bb_by_piece[piece], sq))
            return piece;
    }
    return 0;
}

void Chess::make_move(Move mv)
{
    ep_square = 0;
    int start = mv.start();
    int end = mv.end();
    bool castle = false;

    for (int i = ch_cst::PAWN; i <= ch_cst::KING; i++)
    {
        // remove captured pieces
        *bb_by_piece[i] &= ~(1ull << end);
        *bb_by_color[1 - aci] &= ~(1ull << end);

        // is this type of piece is moving?
        if (!Bitboard::contains_square(*bb_by_piece[i], start))
            continue;

        // remove the old piece
        *bb_by_piece[i] &= ~(1ull << start);
        *bb_by_color[aci] &= ~(1ull << start);
        bb_occ &= ~(1ull << start);

        // place the new piece
        *bb_by_piece[i] |= 1ull << end;
        *bb_by_color[aci] |= 1ull << end;
        bb_occ |= 1ull << end;

        // update ep square
        if (i == ch_cst::PAWN && (start - end) % 16 == 0)
            ep_square = start + directions::PAWN_DIR[aci];

        if (i == ch_cst::KING)
        {
            // handle castles
            // kingside castle
            if (end - start == 2 && castle_rights & 1 << (2 * aci))
            {
                *bb_by_piece[ch_cst::ROOK] ^= 1ull << (start | 0b111);
                *bb_by_piece[ch_cst::ROOK] |= 1ull << (end - 1);
                *bb_by_color[aci] ^= 1ull << (start | 0b111);
                *bb_by_color[aci] |= 1ull << (end - 1);
                bb_occ ^= 1ull << (start | 0b111);
                bb_occ |= 1ull << (end - 1);
            }
            // queenside castle
            else if (end - start == -2 && castle_rights & 2 << (2 * aci))
            {
                *bb_by_piece[ch_cst::ROOK] ^= 1ull << (start & 0b111000);
                *bb_by_piece[ch_cst::ROOK] |= 1ull << (end + 1);
                *bb_by_color[aci] ^= 1ull << (start & 0b111000);
                *bb_by_color[aci] |= 1ull << (end + 1);
                bb_occ ^= 1ull << (start & 0b111000);
                bb_occ |= 1ull << (end + 1);
                
            }
            // update castle rights
            castle_rights &= 3 << (2 * (1 - aci));
        }
        else if (i == ch_cst::ROOK)
        {
            // queenside rook moved
            if (Compass::rank_yindex(start) == 0)
                castle_rights &= ~(2 << 2 * aci);
            // kingside rook moved
            else if (Compass::rank_yindex(start) == 7)
                castle_rights &= ~(1 << 2 * aci);
        }
    }

    ply_counter++;
    aci = 1 - aci;
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

const void Chess::print_board(bool fmt)
{
    std::string board = "";
    for (int sq = 0; sq < 64; sq++)
    {
        for (int color = 0; color < 2; color++)
        {
            for (int piece = ch_cst::PAWN; piece <= ch_cst::KING; piece++)
            {
                if (Bitboard::contains_square(*bb_by_piece[piece] & *bb_by_color[color], sq))
                {
                    board += ch_cst::piece_char[piece | (color << 3)];
                }
            }
        }
        if (board.length() > sq)
            continue;
        board += '.';
    }
    std::cout << std::endl;
    Bitboard::print_binary_string(board, fmt);
}
