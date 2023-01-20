#include "Chess.h"

Chess::State::State()
{
    black_to_move = false;
    ep_square = -1;
    castle_rights = 0b1111;
    build_bitboards(*this);
    std::vector<move> history;
    zhash = hash(*this);
}

Chess::State::State(const State& _ch)
{
    State();
    for (move mv : _ch.history)
        make_move(*this, mv);
}

/*
 * Set up the starting position
 */
void Chess::build_bitboards(State& ch)
{
    ch.bb_white   = 0b1111111111111111;
    ch.bb_black   = ch.bb_white   << (8 * 6);
    ch.bb_pawns   = 0b11111111ull << (8 * 6) | (0b11111111 << 8);
    ch.bb_knights = 0b01000010ull << (8 * 7) | 0b01000010;
    ch.bb_bishops = 0b00100100ull << (8 * 7) | 0b00100100;
    ch.bb_rooks   = 0b10000001ull << (8 * 7) | 0b10000001;
    ch.bb_queens  = 0b00001000ull << (8 * 7) | 0b00001000;
    ch.bb_kings   = 0b00010000ull << (8 * 7) | 0b00010000;
    ch.bb_occ     = ch.bb_white              | ch.bb_black;
}

/*
 * Method to hash a chess position.
 * Typically called once at the start of a game
 * then the hash is incrementally updated during make_move
 */
const U64 Chess::hash(const State& ch)
{
    // who's turn is it?
    U64 h = (ch.black_to_move * TTable::is_black_turn);
    // board representation
    U64 pieces = ch.bb_occ;

    while (pieces)
    {
        // x & -x masks the LS1B
        int sq = 63 - BB::lead_0s(pieces & 0-pieces);
        h ^= TTable::sq_color_type_64x2x6[sq][color_at(ch, sq)][piece_at(ch, sq)];
        // now clear that LS1B
        pieces &= pieces - 1;
    }

    // is there an en passant?
    h ^= ch.ep_square ? TTable::ep_file[ch.ep_square & 7] : 0ull;
    // white castling
    h ^= (ch.castle_rights & 1) * TTable::castle_rights[0][0];
    h ^= ((ch.castle_rights >> 1) & 1) * TTable::castle_rights[0][1];
    // black castling
    h ^= ((ch.castle_rights >> 2) & 1) * TTable::castle_rights[ch_cst::BLACK_INDEX][0];
    h ^= ((ch.castle_rights >> 3) & 1) * TTable::castle_rights[ch_cst::BLACK_INDEX][1];

    return h;
}

/*
 * Method to return the piece type on a square, if any
 * @param sq the square index to check
 * @return the piece type (1 - 6) or 0 if no piece is found
 */
const int Chess::piece_at(const State& ch, int sq)
{
    for (int piece = ch_cst::PAWN; piece <= ch_cst::KING; piece++)
        if (BB::contains_square(*ch.bb_piece[piece], sq))
            return piece;
    return 0;
}

/*
 * Method to return the color of a piece on a square, if any
 * @param sq the square index to check
 * @return 0 for white, 1 for black, -1 if no piece is found
 */
const int Chess::color_at(const State& ch, int sq)
{
    if (BB::contains_square(ch.bb_white, sq)) return ch_cst::WHITE_INDEX;
    if (BB::contains_square(ch.bb_black, sq)) return ch_cst::BLACK_INDEX;
    return -1;
}

void Chess::make_move(State& ch, move mv, bool test)
{
    int start = Move::start(mv);
    int end = Move::end(mv);
    if (ch.ep_square >= 0) ch.zhash ^= TTable::ep_file[Compass::file_xindex(ch.ep_square)];

    // Captured piece
    int piece = piece_at(ch, end);
    if (piece)
    {
        // remove captured pieces
        ch.zhash ^= TTable::sq_color_type_64x2x6[end][1-ch.black_to_move][piece];
        *ch.bb_piece[piece] &= ~(1ull << end);
        *ch.bb_color[1 - ch.black_to_move] &= ~(1ull << end);
    }

    // Moving piece
    piece = Chess::piece_at(ch, start);
    // remove the moving piece
    *ch.bb_piece[piece] &= ~(1ull << start);
    *ch.bb_color[ch.black_to_move] &= ~(1ull << start);
    ch.zhash ^= TTable::sq_color_type_64x2x6[start][ch.black_to_move][piece];

    // place the moving piece
    *ch.bb_piece[Move::promote(mv) ? Move::promote(mv) : piece] |= 1ull << end;
    *ch.bb_color[ch.black_to_move] |= 1ull << end;
    ch.zhash ^= TTable::sq_color_type_64x2x6[end][ch.black_to_move][piece];

    // ep capture
    if (piece == ch_cst::PAWN && end == ch.ep_square)
    {
        ch.bb_pawns &= ~(1ull << (end - directions::PAWN_DIR[ch.black_to_move]));
        *ch.bb_color[1 - ch.black_to_move] &= ~(1ull << (end - directions::PAWN_DIR[ch.black_to_move]));
        ch.zhash ^= TTable::sq_color_type_64x2x6[end - directions::PAWN_DIR[ch.black_to_move]][1-ch.black_to_move][ch_cst::PAWN];
    }

    // update ep square
    if (piece == ch_cst::PAWN && (start - end) % 16 == 0)
    {
        ch.ep_square = start + directions::PAWN_DIR[ch.black_to_move];
        ch.zhash ^= TTable::ep_file[Compass::file_xindex(ch.ep_square)];
    } else ch.ep_square = -1;

    // castles, disable castle rights
    if (piece == ch_cst::KING)
    {
        // handle castles
        // kingside castle
        if (end - start == 2 && ch.castle_rights & 1 << (2 * ch.black_to_move))
        {
            *ch.bb_piece[ch_cst::ROOK] &= ~(1ull << (start | 0b111));
            *ch.bb_piece[ch_cst::ROOK] |= 1ull << (end - 1);
            *ch.bb_color[ch.black_to_move] &= ~(1ull << (start | 0b111));
            *ch.bb_color[ch.black_to_move] |= 1ull << (end - 1);
            ch.zhash ^= TTable::sq_color_type_64x2x6[start|0b111][ch.black_to_move][ch_cst::ROOK];
            ch.zhash ^= TTable::sq_color_type_64x2x6[end-1][ch.black_to_move][ch_cst::ROOK];
        }
        // queenside castle
        else if (end - start == -2 && ch.castle_rights & 2 << (2 * ch.black_to_move))
        {
            *ch.bb_piece[ch_cst::ROOK] &= ~(1ull << (start & 0b111000));
            *ch.bb_piece[ch_cst::ROOK] |= 1ull << (end + 1);
            *ch.bb_color[ch.black_to_move] &= ~(1ull << (start & 0b111000));
            *ch.bb_color[ch.black_to_move] |= 1ull << (end + 1);
            ch.zhash ^= TTable::sq_color_type_64x2x6[start&0b111000][ch.black_to_move][ch_cst::ROOK];
            ch.zhash ^= TTable::sq_color_type_64x2x6[end+1][ch.black_to_move][ch_cst::ROOK];
        }
        // update castle rights
        ch.zhash ^= ch.castle_rights & 1 << 2 * ch.black_to_move ? TTable::castle_rights[ch.black_to_move][0] : 0ull;
        ch.zhash ^= ch.castle_rights & 2 << 2 * ch.black_to_move ? TTable::castle_rights[ch.black_to_move][1] : 0ull;
        ch.castle_rights &= ~(3 << (2 * ch.black_to_move));
    }
    else if (piece == ch_cst::ROOK)
    {
        // queenside rook moved
        if (ch.castle_rights & 1 << 2 * ch.black_to_move && Compass::rank_yindex(start) == 0)
        {
            ch.zhash ^= TTable::castle_rights[ch.black_to_move][1];
            ch.castle_rights &= ~(2 << 2 * ch.black_to_move);
        }
        // kingside rook moved
        else if (ch.castle_rights & 1 << 2 * ch.black_to_move && Compass::rank_yindex(start) == 7)
        {
            ch.zhash ^= TTable::castle_rights[ch.black_to_move][0];
            ch.castle_rights &= ~(1 << 2 * ch.black_to_move);
        }
    }

    ch.bb_occ = ch.bb_white | ch.bb_black;
    ch.black_to_move = !ch.black_to_move;
    ch.zhash ^= TTable::is_black_turn;
    ch.history.push_back(mv);
}

void Chess::unmake_move(State& ch, int undos)
{
    std::vector<move> temp = ch.history;
    ch.history.clear();
    ch.black_to_move = 0;
    ch.ep_square = -1;
    ch.castle_rights = 0b1111;
    build_bitboards(ch);
    hash(ch);
    for (int i = 0; i < (int) temp.size() - undos; i++)
        make_move(ch, temp[i]);
}

const int Chess::repetitions(const State& ch)
{
    int count = 0;
    State test;
    build_bitboards(test);
    for (move m : ch.history)
    {
        count += (ch.zhash == test.zhash);
        make_move(test, m);
    }
    return count + (ch.zhash == test.zhash);
}

const void Chess::print_board(const State& ch, bool fmt)
{
    std::string board = "";
    for (int sq = 0; sq < 64; sq++)
    {
        for (int color = 0; color < 2; color++)
            for (int piece = ch_cst::PAWN; piece <= ch_cst::KING; piece++)
                if (BB::contains_square(*ch.bb_piece[piece] & *ch.bb_color[color], sq))
                    board += ch_cst::piece_char[piece | (color << 3)];
        if ((int) board.length() > sq) continue;
        else if (sq == ch.ep_square) board += "e";
        else if (Compass::file_xindex(sq) % 2 == Compass::rank_yindex(sq) % 2) board += ".";
        else board += " ";
    }
    BB::print_binary_string(board, fmt);
}
