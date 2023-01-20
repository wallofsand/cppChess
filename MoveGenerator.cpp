#include "MoveGenerator.h"

void MoveGenerator::init(bool test)
{
    op_attack_mask = gen_op_attack_mask(test);
    checks_exist(test);
    pinned_pieces = find_pins(test);
}

/*
 * Method to determine whether a game has ended.
 * Calls init(): we use gen_moves().size() to determine state.
 * @return true if stalemate, checkmate
 * @return false if any legal moves exist
 */
bool MoveGenerator::is_game_over(bool test)
{
    init(test);
    return gen_moves().size() == 0 || Chess::repetitions(ch) > 2;
}

void MoveGenerator::checks_exist(bool test)
{
    using namespace directions;
    U64 king = ch.bb_kings & *ch.bb_color[ch.black_to_move];
    in_check = false;
    in_double_check = false;
    check_ray = 0ull;
    if (!(king & op_attack_mask))
        return;
    U64 op = *ch.bb_color[1-ch.black_to_move];
    int ksq = find_king(ch.black_to_move);

    // knights
    U64 attackers = ch.bb_knights & op & Compass::knight_attacks[ksq];
    if (attackers)
    {
        check_ray = attackers & 0-attackers;
        check_method();
    }

    // pawns
    attackers = BB::gen_shift(king & BB::NOT_H_FILE, DIRS[4 + 2 * (ch.black_to_move)]) | BB::gen_shift(king & BB::NOT_A_FILE, DIRS[5 + 2 * (ch.black_to_move)]);
    if (attackers & op & ch.bb_pawns)
    {
        check_ray = attackers & ch.bb_pawns & op;
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
U64 MoveGenerator::find_pins(bool test)
{
    // active king
    U64 king = ch.bb_kings & *ch.bb_color[ch.black_to_move];
    // enemy sliding pieces
    U64 en_rooks = (ch.bb_rooks | ch.bb_queens) & *ch.bb_color[1 - ch.black_to_move];
    U64 en_bishops = (ch.bb_bishops | ch.bb_queens) & *ch.bb_color[1 - ch.black_to_move];

    // North
    U64 ray = BB::nort_attacks(king, ~*ch.bb_color[1 - ch.black_to_move]);
    U64 pinned = (ray & en_rooks && BB::num_bits_flipped(ray & *ch.bb_color[ch.black_to_move]) == 1)
            ? ray & ray & *ch.bb_color[ch.black_to_move] : 0ull;
    // East
    ray = BB::east_attacks(king, ~*ch.bb_color[1 - ch.black_to_move]);
    pinned |= (ray & en_rooks && BB::num_bits_flipped(ray & *ch.bb_color[ch.black_to_move]) == 1)
            ? ray & *ch.bb_color[ch.black_to_move] : 0ull;
    // South
    ray = BB::sout_attacks(king, ~*ch.bb_color[1 - ch.black_to_move]);
    pinned |= (ray & en_rooks && BB::num_bits_flipped(ray & *ch.bb_color[ch.black_to_move]) == 1)
            ? ray & *ch.bb_color[ch.black_to_move] : 0ull;
    // West
    ray = BB::west_attacks(king, ~*ch.bb_color[1 - ch.black_to_move]);
    pinned |= (ray & en_rooks && BB::num_bits_flipped(ray & *ch.bb_color[ch.black_to_move]) == 1)
            ? ray & *ch.bb_color[ch.black_to_move] : 0ull;
    // NorthEast
    ray = BB::NoEa_attacks(king, ~*ch.bb_color[1 - ch.black_to_move]);
    pinned |= (ray & en_bishops && BB::num_bits_flipped(ray & *ch.bb_color[ch.black_to_move]) == 1)
            ? ray & *ch.bb_color[ch.black_to_move] : 0ull;
    // NorthWest
    ray = BB::NoWe_attacks(king, ~*ch.bb_color[1 - ch.black_to_move]);
    pinned |= (ray & en_bishops && BB::num_bits_flipped(ray & *ch.bb_color[ch.black_to_move]) == 1)
            ? ray & *ch.bb_color[ch.black_to_move] : 0ull;
    // SouthEast
    ray = BB::SoEa_attacks(king, ~*ch.bb_color[1 - ch.black_to_move]);
    pinned |= (ray & en_bishops && BB::num_bits_flipped(ray & *ch.bb_color[ch.black_to_move]) == 1)
            ? ray & *ch.bb_color[ch.black_to_move] : 0ull;
    // SouthWest
    ray = BB::SoWe_attacks(king, ~*ch.bb_color[1 - ch.black_to_move]);
    pinned |= (ray & en_bishops && BB::num_bits_flipped(ray & *ch.bb_color[ch.black_to_move]) == 1)
            ? ray & *ch.bb_color[ch.black_to_move] : 0ull;

    // if (test && pinned)
    // {
    //     std::cout << "Pins exist!" << std::endl;
    //     BB::print_U64(pinned, "pinned");
    // }

    return pinned;
}

/*
 * Method to get the opponents' attack mask
 * @return the bitboard of squares attacked by opponent's pieces
 */
U64 MoveGenerator::gen_op_attack_mask(bool test)
{
    U64 mask = Compass::king_attacks[find_king(1-ch.black_to_move)];
    U64 op = *ch.bb_color[1-ch.black_to_move];
    U64 empty = ~ch.bb_occ | (ch.bb_kings & *ch.bb_color[ch.black_to_move]);

    // pawn attacks
    U64 pattacksWest = ch.black_to_move ? BB::NoWe_shift_one(ch.bb_pawns & op) : BB::SoWe_shift_one(ch.bb_pawns & op);
    U64 pattacksEast = ch.black_to_move ? BB::NoEa_shift_one(ch.bb_pawns & op) : BB::SoEa_shift_one(ch.bb_pawns & op);
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
 * @return an unsorted vector of moves
 */
std::vector<move> MoveGenerator::gen_moves(bool test)
{
    init(test);

    // pawns
    std::vector<move> moves = gen_pawn_moves(test);

    // rooks and queens
    U64 ss = (ch.bb_rooks | ch.bb_queens) & *ch.bb_color[ch.black_to_move];
    if (!in_double_check) while (ss)
    {
        // x & -x masks the LS1B
        for (move m : gen_rook_piece_moves(63 - BB::lead_0s(ss & 0-ss), test))
            moves.push_back(m);
        // now clear that LS1B
        ss &= ss - 1;
    }

    // bishops and queens
    ss = (ch.bb_bishops | ch.bb_queens) & *ch.bb_color[ch.black_to_move];
    if (!in_double_check) while (ss)
    {
        // x & -x masks the LS1B
        for (move m : gen_bishop_piece_moves(63 - BB::lead_0s(ss & 0-ss), test))
            moves.push_back(m);
        // now clear that LS1B
        ss &= ss - 1;
    }

    // knights
    ss = ch.bb_knights & *ch.bb_color[ch.black_to_move];
    if (!in_double_check) while (ss)
    {
        // x & -x masks the LS1B
        for (move m : gen_knight_piece_moves(63 - BB::lead_0s(ss & 0-ss), test))
            moves.push_back(m);
        // now clear that LS1B
        ss &= ss - 1;
    }

    // king
    for (move m : gen_king_piece_moves(find_king(ch.black_to_move), test))
        moves.push_back(m);

    return moves;
}

/*
 * Generates legal pawn moves
 * @return an unsorted list of pawn moves
 */
std::vector<move> MoveGenerator::gen_pawn_moves(bool test)
{
    using namespace directions;
    std::vector<move> pawn_moves;
    U64 pawns = ch.bb_pawns & *ch.bb_color[ch.black_to_move];
    if (!pawns || in_double_check)
        return pawn_moves;
    U64 op = *ch.bb_color[1 - ch.black_to_move];

    // pawn captures
    U64 captures_west = (ch.black_to_move) ? BB::SoWe_shift_one(pawns) & (op | 1ull << ch.ep_square) : BB::NoWe_shift_one(pawns) & (op | 1ull << ch.ep_square);
    U64 captures_east = (ch.black_to_move) ? BB::SoEa_shift_one(pawns) & (op | 1ull << ch.ep_square) : BB::NoEa_shift_one(pawns) & (op | 1ull << ch.ep_square);

    // eastern captures
    while (captures_east)
    {
        // x & -x masks the LS1B
        int end_sq = 63 - BB::lead_0s(captures_east & 0-captures_east);
        // now clear that LS1B
        captures_east &= captures_east - 1;
        int start_sq = end_sq - DIRS[4+2*ch.black_to_move];
        if (BB::contains_square(pinned_pieces, start_sq)
                && !BB::contains_square(Compass::ray_square(find_king(ch.black_to_move), start_sq, op), end_sq))
            continue;
        if (Compass::rank_yindex(end_sq) % 7 != 0)
            pawn_moves.push_back(Move::build_move(start_sq, end_sq));
        else // pawn promotions
        {
            pawn_moves.push_back(Move::build_move(start_sq, end_sq, ch_cst::QUEEN));
            pawn_moves.push_back(Move::build_move(start_sq, end_sq, ch_cst::ROOK));
            pawn_moves.push_back(Move::build_move(start_sq, end_sq, ch_cst::BISHOP));
            pawn_moves.push_back(Move::build_move(start_sq, end_sq, ch_cst::KNIGHT));
        }
    }

    // western captures
    while (captures_west)
    {
        // x & -x masks the LS1B
        int end_sq = 63 - BB::lead_0s(captures_west & 0-captures_west);
        // now clear that LS1B
        captures_west &= captures_west - 1;
        int start_sq = end_sq - DIRS[5+2*ch.black_to_move];
        if (BB::contains_square(pinned_pieces, start_sq)
                && !BB::contains_square(Compass::ray_square(find_king(ch.black_to_move), start_sq, op), end_sq))
            continue;
        // if ((~captures_west & 1ull << end_sq)
        //         | (pinned_pieces & 1ull << (end_sq + DIRS[5+2*c.aci]))
        //         | (pin_ray_moves & 1ull << end_sq))
        if (Compass::rank_yindex(end_sq) % 7 != 0)
            pawn_moves.push_back(Move::build_move(start_sq, end_sq));
        else // pawn promotions
        {
            pawn_moves.push_back(Move::build_move(start_sq, end_sq, ch_cst::QUEEN));
            pawn_moves.push_back(Move::build_move(start_sq, end_sq, ch_cst::ROOK));
            pawn_moves.push_back(Move::build_move(start_sq, end_sq, ch_cst::BISHOP));
            pawn_moves.push_back(Move::build_move(start_sq, end_sq, ch_cst::KNIGHT));
        }
    }

    // pawn pushes
    U64 pawn_advances = ch.black_to_move
            ? (pawns >> 8) & ~ch.bb_occ
            : (pawns << 8) & ~ch.bb_occ;
    pawn_advances |= ch.black_to_move
            ? ((pawn_advances & 255ull << 40) >> 8) & ~ch.bb_occ
            : ((pawn_advances & 255ull << 16) << 8) & ~ch.bb_occ;
    while (pawn_advances)
    {
        // x & -x masks the LS1B
        int end_sq = 63 - BB::lead_0s(pawn_advances & 0-pawn_advances);
         // now clear that LS1B
        pawn_advances &= pawn_advances - 1;
        if (BB::contains_square(pawns, end_sq - PAWN_DIR[ch.black_to_move])
                && (!BB::contains_square(pinned_pieces, end_sq - PAWN_DIR[ch.black_to_move])
                    || Compass::file_xindex(find_king(ch.black_to_move)) == Compass::file_xindex(end_sq)))
        {
            if (Compass::rank_yindex(end_sq) % 7 != 0)
                pawn_moves.push_back(Move::build_move(end_sq - PAWN_DIR[ch.black_to_move], end_sq));
            else // pawn promotions
            {
                pawn_moves.push_back(Move::build_move(end_sq - PAWN_DIR[ch.black_to_move], end_sq, ch_cst::QUEEN));
                pawn_moves.push_back(Move::build_move(end_sq - PAWN_DIR[ch.black_to_move], end_sq, ch_cst::ROOK));
                pawn_moves.push_back(Move::build_move(end_sq - PAWN_DIR[ch.black_to_move], end_sq, ch_cst::BISHOP));
                pawn_moves.push_back(Move::build_move(end_sq - PAWN_DIR[ch.black_to_move], end_sq, ch_cst::KNIGHT));
            }
        }
        else if (BB::contains_square(pawns, end_sq - 2 * PAWN_DIR[ch.black_to_move])
                && (!BB::contains_square(pinned_pieces, end_sq - 2 * PAWN_DIR[ch.black_to_move])
                    || Compass::file_xindex(find_king(ch.black_to_move)) == Compass::file_xindex(end_sq)))
            pawn_moves.push_back(Move::build_move(end_sq - 2 * PAWN_DIR[ch.black_to_move], end_sq));
    }

    if (!in_check)
        return pawn_moves;
    std::vector<move> legal_moves;
    for (move m : pawn_moves)
        if (BB::contains_square(check_ray, Move::end(m))
                || (Move::end(m) == ch.ep_square && check_ray & 1ull << (ch.ep_square + directions::PAWN_DIR[1-ch.black_to_move])))
            legal_moves.push_back(m);
    return legal_moves;
}

std::vector<move> MoveGenerator::gen_knight_piece_moves(int start, bool test)
{
    std::vector<move> knight_moves;
    U64 ts = Compass::knight_attacks[start] & ~*ch.bb_color[ch.black_to_move];
    if (BB::contains_square(pinned_pieces, start))
        return knight_moves;

    while (ts)
    {
        // x & -x masks the LS1B
        knight_moves.push_back(Move::build_move(start, 63 - BB::lead_0s(ts & 0-ts)));
        // now clear that LS1B
        ts &= ts - 1;
    }

    if (!in_check)
        return knight_moves;
    std::vector<move> legal_moves;
    for (move m : knight_moves)
    {
        if (check_ray & 1ull << Move::end(m))
            legal_moves.push_back(m);
    }
    return legal_moves;
}

std::vector<move> MoveGenerator::gen_bishop_piece_moves(int start, bool test)
{
    std::vector<move> bishop_moves;

    U64 ss = 1ull << start;
    U64 ts = BB::NoEa_attacks(ss, ~ch.bb_occ);
    ts |= BB::NoWe_attacks(ss, ~ch.bb_occ);
    ts |= BB::SoEa_attacks(ss, ~ch.bb_occ);
    ts |= BB::SoWe_attacks(ss, ~ch.bb_occ);
    ts &= *ch.bb_color[1-ch.black_to_move] | ~ch.bb_occ;

    while (ts)
    {
        // x & -x masks the LS1B
        if (~pinned_pieces & ss || Compass::ray_square(find_king(ch.black_to_move), start) & ts & 0-ts)
            bishop_moves.push_back(Move::build_move(start, 63 - BB::lead_0s(ts & 0-ts)));
        // now clear that LS1B
        ts &= ts - 1;
    }

    if (!in_check)
        return bishop_moves;
    std::vector<move> legal_moves;
    for (move m : bishop_moves)
    {
        if (BB::contains_square(check_ray, Move::end(m)))
            legal_moves.push_back(m);
    }
    return legal_moves;
}

std::vector<move> MoveGenerator::gen_rook_piece_moves(int start, bool test)
{
    std::vector<move> rook_moves;

    U64 ss = 1ull << start;
    U64 ts = BB::nort_attacks(ss, ~ch.bb_occ);
    ts |= BB::sout_attacks(ss, ~ch.bb_occ);
    ts |= BB::east_attacks(ss, ~ch.bb_occ);
    ts |= BB::west_attacks(ss, ~ch.bb_occ);
    ts &= *ch.bb_color[1-ch.black_to_move] | ~ch.bb_occ;

    // ignore pinned pieces
    while (ts)
    {
        // x & -x masks the LS1B
        if (~pinned_pieces & ss || Compass::ray_square(find_king(ch.black_to_move), start) & ts & 0-ts)
            rook_moves.push_back(Move::build_move(start, 63 - BB::lead_0s(ts & 0-ts)));
        // now clear that LS1B
        ts &= ts - 1;
    }

    if (!in_check)
        return rook_moves;
    std::vector<move> legal_moves;
    for (move m : rook_moves)
        if (BB::contains_square(check_ray, Move::end(m)))
            legal_moves.push_back(m);
    return legal_moves;
}

std::vector<move> MoveGenerator::gen_king_piece_moves(int sq, bool test)
{
    std::vector<move> king_moves;
    U64 ts = Compass::king_attacks[sq] & ~*ch.bb_color[ch.black_to_move] & ~op_attack_mask;

    // normal moves
    while (ts)
    {
        // x & -x masks the LS1B
        king_moves.push_back(Move::build_move(sq, 63 - BB::lead_0s(ts & 0-ts)));
        // now clear that LS1B
        ts &= ts - 1;
    }

    // castling
    // wh:QuKi bl:QuKi
    // queenside castle
    if (!in_check && ch.castle_rights & 2 << 2 * ch.black_to_move
            && !BB::contains_square(ch.bb_occ | op_attack_mask, sq - 1)
            && !BB::contains_square(ch.bb_occ | op_attack_mask, sq - 2)
            && !BB::contains_square(ch.bb_occ, sq - 3)
            && BB::contains_square(ch.bb_rooks & *ch.bb_color[ch.black_to_move], sq - 4))
        king_moves.push_back(Move::build_move(sq, sq - 2));
    // kingside castle
    if (!in_check && ch.castle_rights & 1 << 2 * ch.black_to_move
            && !BB::contains_square(ch.bb_occ | op_attack_mask, sq + 1)
            && !BB::contains_square(ch.bb_occ | op_attack_mask, sq + 2)
            && BB::contains_square(ch.bb_rooks & *ch.bb_color[ch.black_to_move], sq + 3))
        king_moves.push_back(Move::build_move(sq, sq + 2));
    return king_moves;
}

/*
 * Method to write the SAN string of a move in the current position.
 * @param mv the Move to write
 * @returns a string of the SAN of the move
 */
std::string MoveGenerator::move_san(move mv)
{
    std::vector<move> moves = gen_moves();
    std::string san = "";
    int start = Move::start(mv), end = Move::end(mv);
    int piece = Chess::piece_at(ch, start);

    // check for ambiguity
    for (move m2 : moves)
    {
        if (Move::start(m2) == start || Move::end(m2) != end || Chess::piece_at(ch, Move::start(m2)) != piece)
            continue;
        if (Compass::file_xindex(Move::start(m2)) != Compass::file_xindex(start))
            san = ch_cst::square_string[start].substr(0, 1) + san;
        if (Compass::rank_yindex(Move::start(m2)) != Compass::rank_yindex(start))
            san += ch_cst::square_string[start].substr(1, 1);
    }

    if (piece != ch_cst::PAWN)
        san = ch_cst::piece_char[piece] + san;

    // captures
    if (BB::contains_square(ch.bb_occ, end) || piece == ch_cst::PAWN && end == ch.ep_square)
    {
        if (piece == ch_cst::PAWN)
            san = ch_cst::square_string[start].substr(0, 1);
        san += "x";
    }
    san += ch_cst::square_string[end];

    // promotions
    if (Move::promote(mv))
    {
        san += "=";
        san += ch_cst::piece_char[Move::promote(mv)];
    }

    // castling
    if (piece == ch_cst::KING && (start - end == 2 || start - end == -2))
    {
        // Kingside castle
        san = "O-O";
        // Queenside castle
        if (start - end == 2)
            san += "-O";
    }

    // check
    Chess::make_move(ch, mv);
    init(false);
    if (in_check)
        san += "+";
    Chess::unmake_move(ch, 1);

    return san;
}
