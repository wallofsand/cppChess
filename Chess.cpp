#include "Chess.h"

Chess::Chess()
{
    black_to_move = false;
    ep_square = -1;
    castle_rights = 0b1111;
    build_bitboards();
    std::vector<move> history;
    zhash = hash();
}

Chess::Chess(const Chess& ch)
{
    black_to_move = false;
    ep_square = -1;
    castle_rights = 0b1111;
    build_bitboards();
    std::vector<move> history;
    zhash = hash();
    for (move mv : ch.history)
        make_move(mv);
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
 * Method to hash a chess position.
 * Typically called once at the start of a game
 * then the hash is incrementally updated during make_move
 */
U64 Chess::hash() const
{
    // who's turn is it?
    U64 h = (black_to_move * TTable::is_black_turn);
    // board representation
    U64 pieces = bb_occ;

    while (pieces)
    {
        // x & -x masks the LS1B
        int sq = 63 - BB::lead_0s(pieces & 0-pieces);
        h ^= TTable::sq_color_type_64x2x6[sq][color_at(sq)][piece_at(sq)];
        // now clear that LS1B
        pieces &= pieces - 1;
    }

    // is there an en passant?
    h ^= ep_square ? TTable::ep_file[ep_square & 7] : 0ull;
    // white castling
    h ^= (castle_rights & 1) * TTable::castle_rights[0][0];
    h ^= ((castle_rights >> 1) & 1) * TTable::castle_rights[0][1];
    // black castling
    h ^= ((castle_rights >> 2) & 1) * TTable::castle_rights[ch_cst::BLACK_INDEX][0];
    h ^= ((castle_rights >> 3) & 1) * TTable::castle_rights[ch_cst::BLACK_INDEX][1];

    return h;
}

/*
 * Method to return the piece type on a square, if any
 * @param sq the square index to check
 * @return the piece type (1 - 6) or 0 if no piece is found
 */
int Chess::piece_at(int sq) const
{
    for (int piece = ch_cst::PAWN; piece <= ch_cst::KING; piece++)
        if (BB::contains_square(*bb_piece[piece], sq))
            return piece;
    return 0;
}

/*
 * Method to return the color of a piece on a square, if any
 * @param sq the square index to check
 * @return 0 for white, 1 for black, -1 if no piece is found
 */
int Chess::color_at(int sq) const
{
    if (BB::contains_square(bb_white, sq)) return ch_cst::WHITE_INDEX;
    if (BB::contains_square(bb_black, sq)) return ch_cst::BLACK_INDEX;
    return -1;
}

void Chess::make_move(move mv, bool test)
{
    int start = Move::start(mv);
    int end = Move::end(mv);
    if (ep_square >= 0) zhash ^= TTable::ep_file[Compass::file_xindex(ep_square)];

    // Captured piece
    int piece = piece_at(end);
    if (piece)
    {
        // remove captured pieces
        zhash ^= TTable::sq_color_type_64x2x6[end][1 - black_to_move][piece];
        *bb_piece[piece] &= ~(1ull << end);
        *bb_color[1 - black_to_move] &= ~(1ull << end);
    }

    // Moving piece
    piece = Chess::piece_at(start);
    // remove the moving piece
    *bb_piece[piece] &= ~(1ull << start);
    *bb_color[black_to_move] &= ~(1ull << start);
    zhash ^= TTable::sq_color_type_64x2x6[start][black_to_move][piece];

    // place the moving piece
    *bb_piece[Move::promote(mv) ? Move::promote(mv) : piece] |= 1ull << end;
    *bb_color[black_to_move] |= 1ull << end;
    zhash ^= TTable::sq_color_type_64x2x6[end][black_to_move][piece];

    // ep capture
    if (piece == ch_cst::PAWN && end == ep_square)
    {
        bb_pawns &= ~(1ull << (end - directions::PAWN_DIR[black_to_move]));
        *bb_color[1 - black_to_move] &= ~(1ull << (end - directions::PAWN_DIR[black_to_move]));
        zhash ^= TTable::sq_color_type_64x2x6[end - directions::PAWN_DIR[black_to_move]][1 - black_to_move][ch_cst::PAWN];
    }

    // update ep square
    if (piece == ch_cst::PAWN && (start - end) % 16 == 0)
    {
        ep_square = start + directions::PAWN_DIR[black_to_move];
        zhash ^= TTable::ep_file[Compass::file_xindex(ep_square)];
    } else ep_square = -1;

    // castles, disable castle rights
    if (piece == ch_cst::KING)
    {
        // handle castles
        // kingside castle
        if (end - start == 2 && castle_rights & 1 << (2 * black_to_move))
        {
            *bb_piece[ch_cst::ROOK] &= ~(1ull << (start | 0b111));
            *bb_piece[ch_cst::ROOK] |= 1ull << (end - 1);
            *bb_color[black_to_move] &= ~(1ull << (start | 0b111));
            *bb_color[black_to_move] |= 1ull << (end - 1);
            zhash ^= TTable::sq_color_type_64x2x6[start | 0b111][black_to_move][ch_cst::ROOK];
            zhash ^= TTable::sq_color_type_64x2x6[end - 1][black_to_move][ch_cst::ROOK];
        }
        // queenside castle
        else if (end - start == -2 && castle_rights & 2 << (2 * black_to_move))
        {
            *bb_piece[ch_cst::ROOK] &= ~(1ull << (start & 0b111000));
            *bb_piece[ch_cst::ROOK] |= 1ull << (end + 1);
            *bb_color[black_to_move] &= ~(1ull << (start & 0b111000));
            *bb_color[black_to_move] |= 1ull << (end + 1);
            zhash ^= TTable::sq_color_type_64x2x6[start&0b111000][black_to_move][ch_cst::ROOK];
            zhash ^= TTable::sq_color_type_64x2x6[end+1][black_to_move][ch_cst::ROOK];
        }
        // update castle rights
        zhash ^= castle_rights & (1 << (2 * black_to_move)) ? TTable::castle_rights[black_to_move][0] : 0ull;
        zhash ^= castle_rights & (2 << (2 * black_to_move)) ? TTable::castle_rights[black_to_move][1] : 0ull;
        castle_rights &= ~(3 << (2 * black_to_move));
    }
    else if (piece == ch_cst::ROOK)
    {
        // queenside rook moved
        if (castle_rights & 1 << 2 * black_to_move && Compass::rank_yindex(start) == 0)
        {
            zhash ^= TTable::castle_rights[black_to_move][1];
            castle_rights &= ~(2 << 2 * black_to_move);
        }
        // kingside rook moved
        else if (castle_rights & 1 << 2 * black_to_move && Compass::rank_yindex(start) == 7)
        {
            zhash ^= TTable::castle_rights[black_to_move][0];
            castle_rights &= ~(1 << 2 * black_to_move);
        }
    }

    bb_occ = bb_white | bb_black;
    black_to_move = !black_to_move;
    zhash ^= TTable::is_black_turn;
    history.push_back(mv);
}

void Chess::unmake_move(int undos)
{
    std::vector<move> temp = history;
    history.clear();
    black_to_move = 0;
    ep_square = -1;
    castle_rights = 0b1111;
    build_bitboards();
    hash();
    for (int i = 0; i < (int) temp.size() - undos; i++)
        make_move(temp[i]);
}

int Chess::repetitions() const
{
    int count = 0;
    Chess test;
    test.build_bitboards();
    for (move mv : history)
    {
        count += (zhash == test.zhash);
        test.make_move(mv);
    }
    return count + (zhash == test.zhash);
}

void Chess::print_board(bool fmt) const
{
    std::string board = "";
    for (int sq = 0; sq < 64; sq++)
    {
        for (int color = 0; color < 2; color++)
            for (int piece = ch_cst::PAWN; piece <= ch_cst::KING; piece++)
                if (BB::contains_square(*bb_piece[piece] & *bb_color[color], sq))
                    board += ch_cst::piece_char[piece | (color << 3)];
        if ((int) board.length() > sq) continue;
        else if (sq == ep_square) board += "e";
        else if (Compass::file_xindex(sq) % 2 == Compass::rank_yindex(sq) % 2) board += ".";
        else board += " ";
    }
    BB::print_binary_string(board, fmt);
}
