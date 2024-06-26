#include "Chess.h"

ch_stk::ChessStack<Chess> Chess::stack;

Chess::Chess() {
    this->black_to_move = false;
    this->ep_square = -1;
    this->castle_rights = 0b1111;
    build_bitboards();
    this->zhash = hash();
}

/*
 * FEN constructor
 * a FEN record contains six fields, separated by a space
 *   1. pieces, ranks 8 to 1, '/' between ranks
 *   2. active color
 *   3. castling availability
 *   4. en passasant target square
 *   5. halfmove clock - number of halfmoves since the last
 *        pawn move or capture. used for the 50-move rule
 *   6. fullmove number, incremented after black's moves
 */
Chess::Chess(const std::string fen) {
    this->castle_rights = 0;
    int loc = 0;

    // field 1: piece locations
    int r = 7;
    int f = 0;
    while (fen[loc] != ' ') {
        while (fen[loc] == '/')
            loc++;
        int sq = r * 8 + f;
        if (49 <= fen[loc] && fen[loc] <= 56) {
            f += fen[loc] - 49;
        } else for (int piece = ch_cst::PAWN; piece <= ch_cst::KING; piece++) {
            if (fen[loc] == ch_cst::piece_char[piece]) {
                // white pieces
                this->bb_white         |= 1ull << sq;
                *this->bb_piece[piece] |= 1ull << sq;
            } else if (fen[loc] == ch_cst::piece_char[piece + 8]) {
                // black pieces
                this->bb_black         |= 1ull << sq;
                *this->bb_piece[piece] |= 1ull << sq;
            }
        }
        f = (f + 1) % 8;
        r = f ? r : r - 1;
        loc++;
    }
    this->bb_occ = bb_white | bb_black;
    loc++;

    // field 2: active color
    this->black_to_move = fen[loc] == 'b';
    loc += 2;

    // field 3: castle availability
    if (fen[loc] != '-')
        while (fen[loc] != ' ') {
            // bl:QuKi wh:QuKi
            if (fen[loc] == 'K')
                this->castle_rights |= 0b0001;
            else if (fen[loc] == 'Q')
                this->castle_rights |= 0b0010;
            else if (fen[loc] == 'k')
                this->castle_rights |= 0b0100;
            else if (fen[loc] == 'q')
                this->castle_rights |= 0b1000;
            loc++;
        }
    else loc++;
    loc++;

    // field 4: ep target square
    if (fen[loc] != '-') {
        this->ep_square = Compass::square_from_string(fen.substr(loc, 2));
        loc += 2;
    } else {
        this->ep_square = -1;
        loc++;
    }
    loc++;

    // field 5: halfmove clock
    this->halfmoves = 0;
    while (fen[loc] != ' ') {
        this->halfmoves *= 10;
        this->halfmoves += fen[loc] - 48;
        loc++;
    }
    loc++;

    // field 6: fullmove clock
    this->fullmoves = 0;
    std::string fmstr = "";
    while (fen[loc] && fen[loc] != ' ') {
        fmstr += fen[loc];
        loc++;
    }
    this->fullmoves = std::stoi(fmstr);

    this->zhash = hash();
}

/*
 * Method to build the FEN string of a position
 * a FEN record contains six fields, separated by a space
 *   1. pieces, ranks 8 to 1, '/' between ranks
 *   2. active color
 *   3. castling availability
 *   4. en passasant target square
 *   5. halfmove clock - number of halfmoves since the last
 *        pawn move or capture. used for the 50-move rule
 *   6. fullmove number, incremented after black's moves
 */
std::string Chess::fen() const {
    std::string fen = "";

    // field 1: piece locations
    for (int r = 7; r > -1; r--) {
        int spaces = 0;
        for (int f = 0; f < 8; f++) {
            int sq = 8 * r + f;
            if (bb_occ & 1ull << sq) {
                if (spaces) {
                    fen += std::to_string(spaces);
                    spaces = 0;
                }
                fen += ch_cst::piece_char[(black_at(sq) << 3) | piece_at(sq)];
            } else spaces++;
        }
        if (spaces) fen += std::to_string(spaces);
        if (r > 0)  fen += '/';
    }
    fen += ' ';

    // field 2: active color
    fen += black_to_move ? "b " : "w ";

    // field 3: castle availability
    if (!castle_rights) fen += '-';
    else {
        if (castle_rights & 1)
            fen += 'K';
        if (castle_rights & 2)
            fen += 'Q';
        if (castle_rights & 4)
            fen += 'k';
        if (castle_rights & 8)
            fen += 'q';
    }
    fen += ' ';

    // field 4: ep target square
    if (ep_square != -1) fen += ch_cst::string_from_square[ep_square] + ' ';
    else                 fen += "- ";

    // field 5: halfmove clock
    fen += std::to_string(halfmoves);
    fen += ' ';

    // field 6: fullmove clock
    fen += std::to_string(fullmoves);
    return fen;
}

// copy constructor without make_move
Chess::Chess(const Chess& _ch) {
    this->black_to_move = _ch.black_to_move;
    this->castle_rights = _ch.castle_rights;
    this->ep_square     = _ch.ep_square;
    this->zhash         = _ch.zhash;
    this->fullmoves     = _ch.fullmoves;
    this->halfmoves     = _ch.halfmoves;
    // copy the bitboards
    this->bb_white      = _ch.bb_white;
    this->bb_black      = _ch.bb_black;
    this->bb_pawns      = _ch.bb_pawns;
    this->bb_knights    = _ch.bb_knights;
    this->bb_bishops    = _ch.bb_bishops;
    this->bb_rooks      = _ch.bb_rooks;
    this->bb_queens     = _ch.bb_queens;
    this->bb_kings      = _ch.bb_kings;
    this->bb_occ        = _ch.bb_occ;
}

/*
 * Set up the starting position
 */
void Chess::build_bitboards() {
    bb_white   = 0b1111111111111111ull;
    bb_black   = bb_white      << 8 * 6;
    bb_pawns   = 0b11111111ull << 8 * 6 | 0b11111111ull << 8;
    bb_knights = 0b01000010ull << 8 * 7 | 0b01000010ull;
    bb_bishops = 0b00100100ull << 8 * 7 | 0b00100100ull;
    bb_rooks   = 0b10000001ull << 8 * 7 | 0b10000001ull;
    bb_queens  = 0b00001000ull << 8 * 7 | 0b00001000ull;
    bb_kings   = 0b00010000ull << 8 * 7 | 0b00010000ull;
    bb_occ     = bb_white               | bb_black;
}

/*
 * Method to hash a chess position.
 * Typically called once at the start of a game
 * then the hash is incrementally updated during make_move
 */
U64 Chess::hash() const {
    // who's turn is it?
    U64 h = black_to_move * TTable::is_black_turn;
    // board representation
    U64 pieces = bb_occ;

    while (pieces) {
        // x & -x masks the LS1B
        int sq = 63 - BB::lz_count(pieces & 0-pieces);
        h ^= TTable::sq_color_type_64x2x6[sq][black_at(sq)][piece_at(sq) - 1];
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
 *         returns 0 if no piece is found
 */
int Chess::piece_at(int sq) const {
    for (int piece = ch_cst::PAWN; piece <= ch_cst::KING; piece++)
        if (*bb_piece[piece] & 1ull << sq)
            return piece;
    return 0;
}

/*
 * Method to find a given king.
 * @param color the color index of the king to search for
 * @return the square index of the king (0-63)
 */
int Chess::find_king(bool is_black) const {
    return 63 - BB::lz_count(bb_kings & *bb_color[is_black]);
}

/*
 * Method to return the color of a piece on a square, if any
 * @param sq the square index to check
 * @return 0 for white, 1 for black, 0 if no piece is found
 */
bool Chess::black_at(int sq) const {
    return bb_black & 1ull << sq;
}

void Chess::push_move(const move mv, bool test) {
    Chess* curr = state();
    Chess* next = new Chess(*curr);
    next->make_move(mv, test);
    // update the game stack
    stack.push(next);
}

void Chess::make_move(const move mv, bool test) {
    halfmoves++;
    int start = Move::start(mv);
    int end = Move::end(mv);

    // Captured piece
    int type = piece_at(end);
    if (type) {
        // remove captured pieces
        zhash ^= TTable::sq_color_type_64x2x6[end][!black_to_move][type - 1];
        *bb_piece[type] &= ~(1ull << end);
        *bb_color[!black_to_move] &= ~(1ull << end);

        // reset the halfmove counter after a capture
        halfmoves = 0;
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

    // Handle en passant captures and update ep square
    zhash ^= (ep_square >= 0) ? TTable::ep_file[Compass::file_xindex(ep_square)] : 0;
    if (type == ch_cst::PAWN) {
        // a pawn moved; reset the halfmove counter
        halfmoves = 0;

        // en passant capture
        bb_pawns ^= end == ep_square ? 1ull << (end - directions::PAWN_DIR[black_to_move]) : 0;
        *bb_color[!black_to_move] ^= end == ep_square ? 1ull << (end - directions::PAWN_DIR[black_to_move]) : 0;
        zhash ^= end == ep_square ? TTable::sq_color_type_64x2x6[end - directions::PAWN_DIR[black_to_move]][!black_to_move][ch_cst::PAWN - 1] : 0;

        // double advance; prepare new en passant square
        ep_square = (start - end) % 16 == 0 ? start + directions::PAWN_DIR[black_to_move] : -1;
        zhash ^= (start - end) % 16 == 0 ? TTable::ep_file[Compass::file_xindex(ep_square)] : 0;
    } else ep_square = -1; // not a pawn move; reset ep_square

    // castles, disable castle rights
    if (type == ch_cst::KING) {
        // handle castles
        if (end - start == 2 && castle_rights & (1 << (2 * black_to_move))) {
            // kingside castle
            *bb_piece[ch_cst::ROOK] &= ~(1ull << (start | 0b111));
            *bb_piece[ch_cst::ROOK] |= 1ull << (end - 1);
            *bb_color[black_to_move] &= ~(1ull << (start | 0b111));
            *bb_color[black_to_move] |= 1ull << (end - 1);
            zhash ^= TTable::sq_color_type_64x2x6[start | 0b111][black_to_move][ch_cst::ROOK - 1];
            zhash ^= TTable::sq_color_type_64x2x6[end - 1][black_to_move][ch_cst::ROOK - 1];
        } else if (end - start == -2 && castle_rights & (2 << (2 * black_to_move))) {
            // queenside castle
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
    } else if (type == ch_cst::ROOK) {
        if (castle_rights & (2 << 2 * black_to_move) && Compass::rank_yindex(start) == 0) {
            // queenside rook moved
            zhash ^= TTable::castle_rights_wb_kq[black_to_move][1];
            castle_rights &= ~(2 << 2 * black_to_move);
        } else if (castle_rights & (1 << 2 * black_to_move) && Compass::rank_yindex(start) == 7) {
            // kingside rook moved
            zhash ^= TTable::castle_rights_wb_kq[black_to_move][0];
            castle_rights &= ~(1 << 2 * black_to_move);
        }
    }

    bb_occ = bb_white | bb_black;
    fullmoves += black_to_move;
    black_to_move = !black_to_move;
    zhash ^= TTable::is_black_turn;
}

void Chess::unmake_move(uint32_t undos) {
    for (uint32_t i = 0; i < undos; i++)
        stack.pop();
}

int Chess::repetitions() const {
    int count = 0;
    ch_stk::StackNode<Chess>* tmp = stack.top;
    while (tmp->next) {
        count += (zhash == tmp->pos->zhash);
        tmp = tmp->next;
    }
    return count;
}

void Chess::print_board(bool fmt) const {
    std::string board = "";
    for (int sq = 0; sq < 64; sq++) {
        if ((bb_black & bb_pawns) & 1ull << sq)
            board += 'o';
        else if (bb_occ & 1ull << sq)
            board += ch_cst::piece_char[(black_at(sq) << 3) | piece_at(sq)];
        else if (sq == ep_square) board += "e";
        else if (Compass::file_xindex(sq) % 2 == Compass::rank_yindex(sq) % 2) board += ".";
        else board += " ";
    }
    BB::print_binary_string(board, fmt);
}
