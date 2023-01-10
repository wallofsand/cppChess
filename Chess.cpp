#include "Chess.h"

Chess::Chess()
{
    aci = 0;
    ply_counter = 0;
    ep_square = -1;
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

/*
 * Method to return the piece type on a square, if any
 * @param sq the square index to check
 * @return the piece type (1 - 6) or 0 if no piece is found
 */
int Chess::piece_at(int sq)
{
    for (int piece = ch_cst::PAWN; piece <= ch_cst::KING; piece++)
    {
        if (BB::contains_square(*bb_piece[piece], sq))
            return piece;
    }
    return 0;
}

/*
 * Method to return the color of a piece on a square, if any
 * @param sq the square index to check
 * @return 0 for white, 1 for black, -1 if no piece is found
 */
int Chess::color_at(int sq)
{
    if (BB::contains_square(bb_white, sq)) return ch_cst::WHITE_INDEX;
    if (BB::contains_square(bb_black, sq)) return ch_cst::BLACK_INDEX;
    return -1;
}

void Chess::make_move(Move mv)
{
    int start = mv.start();
    int end = mv.end();
    bool castle = false;

    for (int piece = ch_cst::PAWN; piece <= ch_cst::KING; piece++)
    {
        // remove captured pieces
        *bb_piece[piece] &= ~(1ull << end);
        *bb_color[1 - aci] &= ~(1ull << end);

        // is this type of piece is moving?
        if (!BB::contains_square(*bb_piece[piece], start))
            continue;

        // remove the old piece
        *bb_piece[piece] &= ~(1ull << start);
        *bb_color[aci] &= ~(1ull << start);

        // place the new piece
        *bb_piece[piece] |= 1ull << end;
        *bb_color[aci] |= 1ull << end;

        // ep capture
        if (piece == ch_cst::PAWN && end == ep_square)
        {
            bb_pawns &= ~(1ull << (end - directions::PAWN_DIR[aci]));
            *bb_color[1 - aci] &= ~(1ull << (end - directions::PAWN_DIR[aci]));
        }

        // update ep square
        if (piece == ch_cst::PAWN && (start - end) % 16 == 0)
            ep_square = start + directions::PAWN_DIR[aci];
        else ep_square = -1;

        if (piece == ch_cst::KING)
        {
            // handle castles
            // kingside castle
            if (end - start == 2 && castle_rights & 1 << (2 * aci))
            {
                *bb_piece[ch_cst::ROOK] &= ~(1ull << (start | 0b111));
                *bb_piece[ch_cst::ROOK] |= 1ull << (end - 1);
                *bb_color[aci] &= ~(1ull << (start | 0b111));
                *bb_color[aci] |= 1ull << (end - 1);
            }
            // queenside castle
            else if (end - start == -2 && castle_rights & 2 << (2 * aci))
            {
                *bb_piece[ch_cst::ROOK] &= ~(1ull << (start & 0b111000));
                *bb_piece[ch_cst::ROOK] |= 1ull << (end + 1);
                *bb_color[aci] &= ~(1ull << (start & 0b111000));
                *bb_color[aci] |= 1ull << (end + 1);
            }
            // update castle rights
            castle_rights &= 3 << (2 * (1 - aci));
        }
        else if (piece == ch_cst::ROOK)
        {
            // queenside rook moved
            if (Compass::rank_yindex(start) == 0)
                castle_rights &= ~(2 << 2 * aci);
            // kingside rook moved
            else if (Compass::rank_yindex(start) == 7)
                castle_rights &= ~(1 << 2 * aci);
        }
    bb_occ = bb_white | bb_black;
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
    ep_square = -1;
    ply_counter = 0;
    castle_rights = 0b1111;
    build_bitboards();
    for (int i = 0; i < (int) temp.size() - undos; i++)
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
                if (BB::contains_square(*bb_piece[piece] & *bb_color[color], sq))
                    board += ch_cst::piece_char[piece | (color << 3)];
        }
        if ((int) board.length() > sq) continue;
        else if (sq == ep_square) board += "e";
        else if (Compass::file_xindex(sq) % 2 == Compass::rank_yindex(sq) % 2) board += ".";
        else board += " ";
    }
    std::cout << std::endl;
    BB::print_binary_string(board, fmt);
}
