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

// copy constructor without make_move
Chess::Chess(const Chess& ch)
{
    this->black_to_move = ch.black_to_move;
    this->ep_square     = ch.ep_square;
    this->castle_rights = ch.castle_rights;
    this->history       = ch.history;
    this->zhash         = ch.zhash;
    // copy the bitboards
    this->bb_white   = bb_white;
    this->bb_black   = bb_black;
    this->bb_pawns   = bb_pawns;
    this->bb_knights = bb_knights;
    this->bb_bishops = bb_bishops;
    this->bb_rooks   = bb_rooks;
    this->bb_queens  = bb_queens;
    this->bb_kings   = bb_kings;
    this->bb_occ     = bb_occ;
}

/***********************************
 *       OLD COPY CONSTRUCOR       *
 ***********************************
 * Chess::Chess(const Chess& ch)   *
 * {                               *
 *     black_to_move = false;      *
 *     ep_square = -1;             *
 *     castle_rights = 0b1111;     *
 *     build_bitboards();          *
 *     std::vector<move> history;  *
 *     zhash = hash();             *
 *     for (move mv : ch.history)  *
 *         make_move(mv);          *
 * }                               *
 **********************************/

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
    U64 h = black_to_move * TTable::is_black_turn;
    // board representation
    U64 pieces = bb_occ;

    while (pieces)
    {
        // x & -x masks the LS1B
        int sq = 63 - BB::lead_0s(pieces & 0-pieces);
        h ^= TTable::sq_color_type_64x2x6[sq][color_at(sq)][piece_at(sq) - 1];
        // now clear that LS1B
        pieces &= pieces - 1;
    }

    // is there an en passant?
    h ^= (ep_square >= 0 ? TTable::ep_file[ep_square & 7] : 0ull);
    // white castling
    h ^= (castle_rights & 1)        * TTable::castle_rights_wb_kq[0][0];
    h ^= ((castle_rights >> 1) & 1) * TTable::castle_rights_wb_kq[0][1];
    // black castling
    h ^= ((castle_rights >> 2) & 1) * TTable::castle_rights_wb_kq[1][0];
    h ^= ((castle_rights >> 3) & 1) * TTable::castle_rights_wb_kq[1][1];

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
    prev_hash.push_back(zhash);

    // Captured piece
    int type = piece_at(end);
    if (type)
    {
        // remove captured pieces
        zhash ^= TTable::sq_color_type_64x2x6[end][!black_to_move][type - 1];
        *bb_piece[type] &= ~(1ull << end);
        *bb_color[!black_to_move] &= ~(1ull << end);
    }

    // Moving piece
    type = Chess::piece_at(start);
    // remove the moving piece
    *bb_piece[type] &= ~(1ull << start);
    *bb_color[black_to_move] &= ~(1ull << start);
    zhash ^= TTable::sq_color_type_64x2x6[start][black_to_move][type - 1];

    // place the moving piece
    *bb_piece[Move::promote(mv) ? Move::promote(mv) : type] |= 1ull << end;
    *bb_color[black_to_move] |= 1ull << end;
    zhash ^= TTable::sq_color_type_64x2x6[end][black_to_move][(Move::promote(mv) ? Move::promote(mv) : type) - 1];

    // ep capture
    if (type == ch_cst::PAWN && end == ep_square)
    {
        bb_pawns &= ~(1ull << (end - directions::PAWN_DIR[black_to_move]));
        *bb_color[!black_to_move] &= ~(1ull << (end - directions::PAWN_DIR[black_to_move]));
        zhash ^= TTable::sq_color_type_64x2x6[end - directions::PAWN_DIR[black_to_move]][!black_to_move][ch_cst::PAWN - 1];
    }

    // clear ep square
    if (ep_square >= 0) zhash ^= TTable::ep_file[Compass::file_xindex(ep_square)];
    // update ep square
    if (type == ch_cst::PAWN && (start - end) % 16 == 0)
    {
        ep_square = start + directions::PAWN_DIR[black_to_move];
        zhash ^= TTable::ep_file[Compass::file_xindex(ep_square)];
    } else ep_square = -1;

    // castles, disable castle rights
    if (type == ch_cst::KING)
    {
        // handle castles
        // kingside castle
        if (end - start == 2 && castle_rights & (1 << (2 * black_to_move)))
        {
            *bb_piece[ch_cst::ROOK] &= ~(1ull << (start | 0b111));
            *bb_piece[ch_cst::ROOK] |= 1ull << (end - 1);
            *bb_color[black_to_move] &= ~(1ull << (start | 0b111));
            *bb_color[black_to_move] |= 1ull << (end - 1);
            zhash ^= TTable::sq_color_type_64x2x6[start | 0b111][black_to_move][ch_cst::ROOK - 1];
            zhash ^= TTable::sq_color_type_64x2x6[end - 1][black_to_move][ch_cst::ROOK - 1];
        }
        // queenside castle
        else if (end - start == -2 && castle_rights & (2 << (2 * black_to_move)))
        {
            *bb_piece[ch_cst::ROOK] &= ~(1ull << (start & 0b111000));
            *bb_piece[ch_cst::ROOK] |= 1ull << (end + 1);
            *bb_color[black_to_move] &= ~(1ull << (start & 0b111000));
            *bb_color[black_to_move] |= 1ull << (end + 1);
            zhash ^= TTable::sq_color_type_64x2x6[start & 0b111000][black_to_move][ch_cst::ROOK - 1];
            zhash ^= TTable::sq_color_type_64x2x6[end + 1][black_to_move][ch_cst::ROOK - 1];
        }
        // update castle rights
        zhash ^= castle_rights & (1 << (2 * black_to_move)) ? TTable::castle_rights_wb_kq[black_to_move][0] : 0ull;
        zhash ^= castle_rights & (2 << (2 * black_to_move)) ? TTable::castle_rights_wb_kq[black_to_move][1] : 0ull;
        castle_rights &= ~(3 << (2 * black_to_move));
    }
    else if (type == ch_cst::ROOK)
    {
        // queenside rook moved
        if (castle_rights & (2 << 2 * black_to_move) && Compass::rank_yindex(start) == 0)
        {
            zhash ^= TTable::castle_rights_wb_kq[black_to_move][1];
            castle_rights &= ~(2 << 2 * black_to_move);
        }
        // kingside rook moved
        else if (castle_rights & (1 << 2 * black_to_move) && Compass::rank_yindex(start) == 7)
        {
            zhash ^= TTable::castle_rights_wb_kq[black_to_move][0];
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
    prev_hash.clear();
    black_to_move = false;
    ep_square = -1;
    castle_rights = 0b1111;
    build_bitboards();
    zhash = hash();
    for (int i = 0; i < (int) temp.size() - undos; i++)
        make_move(temp[i]);
}

int Chess::repetitions() const
{
    uint8_t count = 1;
    for (U64 h : prev_hash)
        count += (zhash == h);
    return count;
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
