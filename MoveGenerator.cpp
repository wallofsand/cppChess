#include "MoveGenerator.h"

MoveGenerator::MoveGenerator(Chess& ch) : ch(ch) { }

void MoveGenerator::init(bool test)
{
    op_attack_mask = gen_op_attack_mask(test);
    checks_exist(test);
    find_pins(test);
}

/*
 * Method to determine whether a game has ended.
 * Must call AFTER init(): we use gen_moves().size() to determine state.
 * @return true if stalemate, checkmate
 * @return false if any legal moves exist
 */
bool MoveGenerator::is_game_over(bool test)
{
    return gen_moves(test).size() == 0;
}

void MoveGenerator::checks_exist(bool test)
{
    using namespace directions;
    U64 king = ch.bb_kings & *ch.bb_color[ch.aci];
    in_check = false;
    in_double_check = false;
    check_ray = 0;
    if (!(king & op_attack_mask))
        return;
    U64 op = *ch.bb_color[1-ch.aci];
    int ksq = find_king(ch.aci);

    // knights
    U64 attackers = ch.bb_knights & op & Compass::knight_attacks[ksq];
    if (attackers)
    {
        check_ray |= attackers & 0-attackers;
        check_method();
    }

    // pawns
    attackers = BB::gen_shift(king & BB::NOT_H_FILE, DIRS[4 + 2 * (ch.aci)]) | BB::gen_shift(king & BB::NOT_A_FILE, DIRS[5 + 2 * (ch.aci)]);
    if (attackers & op & ch.bb_pawns)
    {
        check_ray |= 1ull << (63 - BB::lead_0s(attackers));
        check_method();
    }

    // bishops
    attackers = BB::NoEa_attacks(king, ~ch.bb_occ);
    if (attackers & (ch.bb_queens | ch.bb_bishops) & op)
    {
        check_ray = attackers;
        check_method();
    }
    attackers = BB::NoWe_attacks(king, ~ch.bb_occ);
    if (attackers & (ch.bb_queens | ch.bb_bishops) & op)
    {
        check_ray = attackers;
        check_method();
    }
    attackers = BB::SoEa_attacks(king, ~ch.bb_occ);
    if (attackers & (ch.bb_queens | ch.bb_bishops) & op)
    {
        check_ray = attackers;
        check_method();
    }
    attackers = BB::SoWe_attacks(king, ~ch.bb_occ);
    if (attackers & (ch.bb_queens | ch.bb_bishops) & op)
    {
        check_ray = attackers;
        check_method();
    }

    // rooks
    attackers = BB::nort_attacks(king, ~ch.bb_occ);
    if (attackers & (ch.bb_queens | ch.bb_rooks) & op)
    {
        check_ray = attackers;
        check_method();
    }
    attackers = BB::sout_attacks(king, ~ch.bb_occ);
    if (attackers & (ch.bb_queens | ch.bb_rooks) & op)
    {
        check_ray = attackers;
        check_method();
    }
    attackers = BB::east_attacks(king, ~ch.bb_occ);
    if (attackers & (ch.bb_queens | ch.bb_rooks) & op)
    {
        check_ray = attackers;
        check_method();
    }
    attackers = BB::west_attacks(king, ~ch.bb_occ);
    if (attackers & (ch.bb_queens | ch.bb_rooks) & op)
    {
        check_ray = attackers;
        check_method();
    }
}

void MoveGenerator::check_method()
{
    if (in_check)
    {
        in_double_check = true;
        return;
    }
    in_check = true;
}

// setup method to get pins in a position
// modifies a bitboard of squares containing pinning pieces
void MoveGenerator::find_pins(bool test)
{
    // pieces that are pinned to the king
    pinned_pieces = 0;

    U64 en_rooks = (ch.bb_rooks | ch.bb_queens) & *ch.bb_color[1 - ch.aci];
    U64 en_bishops = (ch.bb_bishops | ch.bb_queens) & *ch.bb_color[1 - ch.aci];
    U64 king = ch.bb_kings & *ch.bb_color[ch.aci];

    U64 ray = BB::nort_attacks(king, ~*ch.bb_color[1 - ch.aci]);
    if (ray & en_rooks && BB::num_bits_flipped(ray & *ch.bb_color[ch.aci]) == 1)
        pinned_pieces |= ray & *ch.bb_color[ch.aci];
    ray = BB::east_attacks(king, ~*ch.bb_color[1 - ch.aci]);
    if (ray & en_rooks && BB::num_bits_flipped(ray & *ch.bb_color[ch.aci]) == 1)
        pinned_pieces |= ray & *ch.bb_color[ch.aci];
    ray = BB::sout_attacks(king, ~*ch.bb_color[1 - ch.aci]);
    if (ray & en_rooks && BB::num_bits_flipped(ray & *ch.bb_color[ch.aci]) == 1)
        pinned_pieces |= ray & *ch.bb_color[ch.aci];
    ray = BB::west_attacks(king, ~*ch.bb_color[1 - ch.aci]);
    if (ray & en_rooks && BB::num_bits_flipped(ray & *ch.bb_color[ch.aci]) == 1)
        pinned_pieces |= ray & *ch.bb_color[ch.aci];
    ray = BB::NoEa_attacks(king, ~*ch.bb_color[1 - ch.aci]);
    if (ray & en_bishops && BB::num_bits_flipped(ray & *ch.bb_color[ch.aci]) == 1)
        pinned_pieces |= ray & *ch.bb_color[ch.aci];
    ray = BB::NoWe_attacks(king, ~*ch.bb_color[1 - ch.aci]);
    if (ray & en_bishops && BB::num_bits_flipped(ray & *ch.bb_color[ch.aci]) == 1)
        pinned_pieces |= ray & *ch.bb_color[ch.aci];
    ray = BB::SoEa_attacks(king, ~*ch.bb_color[1 - ch.aci]);
    if (ray & en_bishops && BB::num_bits_flipped(ray & *ch.bb_color[ch.aci]) == 1)
        pinned_pieces |= ray & *ch.bb_color[ch.aci];
    ray = BB::SoWe_attacks(king, ~*ch.bb_color[1 - ch.aci]);
    if (ray & en_bishops && BB::num_bits_flipped(ray & *ch.bb_color[ch.aci]) == 1)
        pinned_pieces |= ray & *ch.bb_color[ch.aci];

    // if (test && pinned_pieces)
    // {
    //     std::cout << "Pins exist!" << std::endl;
    //     BB::print_U64(pinned_pieces, "pinned_pieces");
    // }
}

/*
 * Method to get the opponents' attack mask
 * @return the bitboard of squares attacked by opponent's pieces
 */
U64 MoveGenerator::gen_op_attack_mask(bool test)
{
    U64 mask = Compass::king_attacks[find_king(1-ch.aci)];
    U64 op = *ch.bb_color[1-ch.aci];
    U64 empty = ~ch.bb_occ | (ch.bb_kings & *ch.bb_color[ch.aci]);

    // pawn attacks
    U64 pattacksWest = ch.aci ? BB::NoWe_shift_one(ch.bb_pawns & op) : BB::SoWe_shift_one(ch.bb_pawns & op);
    U64 pattacksEast = ch.aci ? BB::NoEa_shift_one(ch.bb_pawns & op) : BB::SoEa_shift_one(ch.bb_pawns & op);
    mask |= pattacksEast;
    mask |= pattacksWest;

    // rooks & queens
    U64 attackers = (ch.bb_rooks | ch.bb_queens) & op;
    mask |= BB::nort_attacks(attackers, empty);
    mask |= BB::sout_attacks(attackers, empty);
    mask |= BB::east_attacks(attackers, empty);
    mask |= BB::west_attacks(attackers, empty);

    // bishops & queens
    attackers = (ch.bb_bishops | ch.bb_queens) & op;
    mask |= BB::NoEa_attacks(attackers, empty);
    mask |= BB::NoWe_attacks(attackers, empty);
    mask |= BB::SoEa_attacks(attackers, empty);
    mask |= BB::SoWe_attacks(attackers, empty);

    // knights
    attackers = ch.bb_knights & op;
    while (attackers)
    {
        // x & -x masks the LS1B
        mask |= Compass::knight_attacks[63 - BB::lead_0s(attackers & 0-attackers)];
        // now clear that LS1B
        attackers &= attackers - 1;
    }
    return mask;
}

/*
 * Method to find a given king.
 * @param color the color index of the king to search for
 * @return the square index of the king (0-63)
 */
int MoveGenerator::find_king(int color)
{
    return 63 - BB::lead_0s(ch.bb_kings & *ch.bb_color[color]);
}

/*
 * Method to get the legal moves in a position.
 * Remeber to call init() first.
 * @return an unsorted vector of moves
 */
std::vector<Move> MoveGenerator::gen_moves(bool test)
{
    init(test);

    // pawns
    std::vector<Move> moves = gen_pawn_moves(test);

    // rooks and queens
    U64 ss = (ch.bb_rooks | ch.bb_queens) & *ch.bb_color[ch.aci];
    if (!in_double_check) while (ss)
    {
        // x & -x masks the LS1B
        for (Move m : gen_rook_piece_moves(63 - BB::lead_0s(ss & 0-ss), test))
            moves.push_back(m);
        // now clear that LS1B
        ss &= ss - 1;
    }

    // bishops and queens
    ss = (ch.bb_bishops | ch.bb_queens) & *ch.bb_color[ch.aci];
    if (!in_double_check) while (ss)
    {
        // x & -x masks the LS1B
        for (Move m : gen_bishop_piece_moves(63 - BB::lead_0s(ss & 0-ss), test))
            moves.push_back(m);
        // now clear that LS1B
        ss &= ss - 1;
    }

    // knights
    ss = ch.bb_knights & *ch.bb_color[ch.aci];
    if (!in_double_check) while (ss)
    {
        // x & -x masks the LS1B
        for (Move m : gen_knight_piece_moves(63 - BB::lead_0s(ss & 0-ss), test))
            moves.push_back(m);
        // now clear that LS1B
        ss &= ss - 1;
    }

    // king
    for (Move m : gen_king_piece_moves(find_king(ch.aci), test))
        moves.push_back(m);

    return moves;
}

/*
 * Generates legal pawn moves
 * @return an unsorted list of pawn moves
 */
std::vector<Move> MoveGenerator::gen_pawn_moves(bool test)
{
    using namespace directions;
    std::vector<Move> pawn_moves;
    U64 pawns = ch.bb_pawns & *ch.bb_color[ch.aci];
    if (!pawns || in_double_check)
        return pawn_moves;
    U64 op = *ch.bb_color[1 - ch.aci];

    // pawn captures
    U64 captures_west = ch.aci
                        ? BB::SoWe_shift_one(pawns) & (op | 1ull << ch.ep_square)
                        : BB::NoWe_shift_one(pawns) & (op | 1ull << ch.ep_square);
    U64 captures_east = ch.aci
                        ? BB::SoEa_shift_one(pawns) & (op | 1ull << ch.ep_square)
                        : BB::NoEa_shift_one(pawns) & (op | 1ull << ch.ep_square);

    // eastern captures
    for (int end_sq = (ch.aci) ? 0 : 16; captures_east >> end_sq; end_sq++)
    {
        if (!BB::contains_square(captures_east, end_sq))
            continue;
        if (BB::contains_square(pinned_pieces, end_sq - DIRS[4+2*ch.aci])
                && !BB::contains_square(Compass::ray_square(find_king(ch.aci), end_sq), end_sq))
            continue;
        if (Compass::rank_yindex(end_sq) % 7 != 0)
            pawn_moves.push_back(Move(end_sq - DIRS[4+2*ch.aci], end_sq));
        else // pawn promotions
        {
            pawn_moves.push_back(Move(end_sq - DIRS[4+2*ch.aci], end_sq, ch_cst::QUEEN));
            pawn_moves.push_back(Move(end_sq - DIRS[4+2*ch.aci], end_sq, ch_cst::ROOK));
            pawn_moves.push_back(Move(end_sq - DIRS[4+2*ch.aci], end_sq, ch_cst::BISHOP));
            pawn_moves.push_back(Move(end_sq - DIRS[4+2*ch.aci], end_sq, ch_cst::KNIGHT));
        }
    }

    // western captures
    for (int end_sq = (ch.aci) ? 0 : 16; captures_west >> end_sq > 0; end_sq++)
    {
        if (!BB::contains_square(captures_west, end_sq))
            continue;
        if (BB::contains_square(pinned_pieces, end_sq - DIRS[5+2*ch.aci])
                && !BB::contains_square(Compass::ray_square(find_king(ch.aci), end_sq), end_sq))
            continue;
        // if ((~captures_west & 1ull << end_sq)
        //         | (pinned_pieces & 1ull << (end_sq - DIRS[5+2*c.aci]))
        //         | (pin_ray_moves & 1ull << end_sq))
        if (Compass::rank_yindex(end_sq) % 7 != 0)
            pawn_moves.push_back(Move(end_sq - DIRS[5+2*ch.aci], end_sq));
        else // pawn promotions
        {
            pawn_moves.push_back(Move(end_sq - DIRS[5+2*ch.aci], end_sq, ch_cst::QUEEN));
            pawn_moves.push_back(Move(end_sq - DIRS[5+2*ch.aci], end_sq, ch_cst::ROOK));
            pawn_moves.push_back(Move(end_sq - DIRS[5+2*ch.aci], end_sq, ch_cst::BISHOP));
            pawn_moves.push_back(Move(end_sq - DIRS[5+2*ch.aci], end_sq, ch_cst::KNIGHT));
        }
    }

    // pawn moves
    // white to move
    U64 pawn_advances = (pawns << 8) & ~ch.bb_occ;
    pawn_advances |= ((pawn_advances & 255ull << 16) << 8) & ~ch.bb_occ;
    if (ch.aci)
    {
        // black to move
        pawn_advances = (pawns >> 8) & ~ch.bb_occ;
        pawn_advances |= ((pawn_advances & 255ull << 40) >> 8) & ~ch.bb_occ;
    }
    for (int end_sq = (ch.aci) ? 0 : 16; pawn_advances >> end_sq; end_sq++)
    {
        if (!BB::contains_square(pawn_advances, end_sq))
            continue;
        if (BB::contains_square(pawns, end_sq - PAWN_DIR[ch.aci])
                && (!BB::contains_square(pinned_pieces, end_sq - PAWN_DIR[ch.aci])
                    || Compass::file_xindex(find_king(ch.aci)) == Compass::file_xindex(end_sq)))
        {
            if (Compass::rank_yindex(end_sq) % 7 != 0)
                pawn_moves.push_back(Move(end_sq - PAWN_DIR[ch.aci], end_sq));
            else // pawn promotions
            {
                pawn_moves.push_back(Move(end_sq - PAWN_DIR[ch.aci], end_sq, ch_cst::QUEEN));
                pawn_moves.push_back(Move(end_sq - PAWN_DIR[ch.aci], end_sq, ch_cst::ROOK));
                pawn_moves.push_back(Move(end_sq - PAWN_DIR[ch.aci], end_sq, ch_cst::BISHOP));
                pawn_moves.push_back(Move(end_sq - PAWN_DIR[ch.aci], end_sq, ch_cst::KNIGHT));
            }
        }
        else if (BB::contains_square(pawns, end_sq - 2 * PAWN_DIR[ch.aci])
                && (!BB::contains_square(pinned_pieces, end_sq - 2 * PAWN_DIR[ch.aci])
                    || Compass::file_xindex(find_king(ch.aci)) == Compass::file_xindex(end_sq)))
            pawn_moves.push_back(Move(end_sq - 2 * PAWN_DIR[ch.aci], end_sq));
    }

    if (!in_check)
        return pawn_moves;
    std::vector<Move> legal_moves;
    for (Move mv : pawn_moves)
        if (BB::contains_square(check_ray, mv.end())
                || (mv.end() == ch.ep_square && check_ray & 1ull << (ch.ep_square + directions::PAWN_DIR[1-ch.aci])))
            legal_moves.push_back(mv);
    return legal_moves;
}

std::vector<Move> MoveGenerator::gen_knight_piece_moves(int start, bool test)
{
    std::vector<Move> knight_moves;
    U64 ts = Compass::knight_attacks[start] & ~*ch.bb_color[ch.aci];
    if (BB::contains_square(pinned_pieces, start))
        return knight_moves;

    while (ts)
    {
        // x & -x masks the LS1B
        knight_moves.push_back(Move(start, 63 - BB::lead_0s(ts & 0-ts)));
        // now clear that LS1B
        ts &= ts - 1;
    }

    if (!in_check)
        return knight_moves;
    std::vector<Move> legal_moves;
    for (Move m : knight_moves)
    {
        if (check_ray & 1ull << m.end())
            legal_moves.push_back(m);
    }
    return legal_moves;
}

std::vector<Move> MoveGenerator::gen_bishop_piece_moves(int start, bool test)
{
    std::vector<Move> bishop_moves;

    U64 ss = 1ull << start;
    U64 ts = BB::NoEa_attacks(ss, ~ch.bb_occ);
    ts |= BB::NoWe_attacks(ss, ~ch.bb_occ);
    ts |= BB::SoEa_attacks(ss, ~ch.bb_occ);
    ts |= BB::SoWe_attacks(ss, ~ch.bb_occ);
    ts &= *ch.bb_color[1-ch.aci] | ~ch.bb_occ;

    while (ts)
    {
        // x & -x masks the LS1B
        if (~pinned_pieces & ss || Compass::ray_square(find_king(ch.aci), start) & ts & 0-ts)
            bishop_moves.push_back(Move(start, 63 - BB::lead_0s(ts & 0-ts)));
        // now clear that LS1B
        ts &= ts - 1;
    }

    if (!in_check)
        return bishop_moves;
    std::vector<Move> legal_moves;
    for (Move m : bishop_moves)
    {
        if (BB::contains_square(check_ray, m.end()))
            legal_moves.push_back(m);
    }
    return legal_moves;
}

std::vector<Move> MoveGenerator::gen_rook_piece_moves(int start, bool test)
{
    std::vector<Move> rook_moves;

    U64 ss = 1ull << start;
    U64 ts = BB::nort_attacks(ss, ~ch.bb_occ);
    ts |= BB::sout_attacks(ss, ~ch.bb_occ);
    ts |= BB::east_attacks(ss, ~ch.bb_occ);
    ts |= BB::west_attacks(ss, ~ch.bb_occ);
    ts &= *ch.bb_color[1-ch.aci] | ~ch.bb_occ;

    while (ts)
    {
        // x & -x masks the LS1B
        if (~pinned_pieces & ss || Compass::ray_square(find_king(ch.aci), start) & ts & 0-ts)
            rook_moves.push_back(Move(start, 63 - BB::lead_0s(ts & 0-ts)));
        // now clear that LS1B
        ts &= ts - 1;
    }

    if (!in_check)
        return rook_moves;
    std::vector<Move> legal_moves;
    for (Move m : rook_moves)
        if (BB::contains_square(check_ray, m.end()))
            legal_moves.push_back(m);
    return legal_moves;
}

std::vector<Move> MoveGenerator::gen_king_piece_moves(int sq, bool test)
{
    std::vector<Move> king_moves;
    U64 ts = Compass::king_attacks[sq] & ~*ch.bb_color[ch.aci] & ~op_attack_mask;

    // normal moves
    while (ts)
    {
        // x & -x masks the LS1B
        king_moves.push_back(Move(sq, 63 - BB::lead_0s(ts & 0-ts)));
        // now clear that LS1B
        ts &= ts - 1;
    }

    // castling
    // wh:QuKi bl:QuKi
    // queenside castle
    if (!in_check && ch.castle_rights & 2 << 2 * ch.aci
            && !BB::contains_square(ch.bb_occ | op_attack_mask, sq - 1)
            && !BB::contains_square(ch.bb_occ | op_attack_mask, sq - 2)
            && !BB::contains_square(ch.bb_occ, sq - 3)
            && BB::contains_square(ch.bb_rooks & *ch.bb_color[ch.aci], sq - 4))
        king_moves.push_back(Move(sq, sq - 2));
    // kingside castle
    if (!in_check && ch.castle_rights & 1 << 2 * ch.aci
            && !BB::contains_square(ch.bb_occ | op_attack_mask, sq + 1)
            && !BB::contains_square(ch.bb_occ | op_attack_mask, sq + 2)
            && BB::contains_square(ch.bb_rooks & *ch.bb_color[ch.aci], sq + 3))
        king_moves.push_back(Move(sq, sq + 2));
    return king_moves;
}

/*
 * Method to write the SAN string of a move in the current position.
 * @param mv the Move to write
 * @returns a string of the SAN of the move
 */
std::string MoveGenerator::move_san(Move& mv)
{
    using namespace ch_cst;
    std::vector<Move> moves = gen_moves();
    std::string san = "";
    int start = mv.start(), end = mv.end();
    int piece = ch.piece_at(start);

    // check for ambiguity
    for (Move m2 : moves)
    {
        if (m2.start() == start || m2.end() != end || ch.piece_at(m2.start()) != piece)
            continue;
        if (Compass::file_xindex(m2.start()) != Compass::file_xindex(start))
            san = square_string[start].substr(0, 1) + san;
        if (Compass::rank_yindex(m2.start()) != Compass::rank_yindex(start))
            san += square_string[start].substr(1, 1);
    }

    if (piece != PAWN)
        san = piece_char[piece] + san;

    // captures
    if (BB::contains_square(ch.bb_occ, end) || piece == PAWN && end == ch.ep_square)
    {
        if (piece == PAWN)
            san = square_string[start].substr(0, 1);
        san += "x";
    }
    san += square_string[end];

    // castling
    if (piece == KING && (start - end == 2 || start - end == -2))
    {
        // Kingside castle
        san = "O-O";
        // Queenside castle
        if (start - end == 2)
            san += "-O";
    }

    // promotions
    if (mv.promote())
        san += "=" + piece_char[mv.promote()];

    // check
    ch.make_move(mv);
    init(false);
    if (in_check)
        san += "+";
    ch.unmake_move(1);

    return san;
}
