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
    move moves[120] = {};
    gen_moves(moves);
    return !moves[119] || ch.repetitions() > 2;
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

    U64 op = *ch.bb_color[1 - ch.black_to_move];
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
    U64 en_rooks = (ch.bb_rooks | ch.bb_queens) & *ch.bb_color[!ch.black_to_move];
    U64 en_bishops = (ch.bb_bishops | ch.bb_queens) & *ch.bb_color[!ch.black_to_move];

    // North
    U64 ray = BB::nort_attacks(king, ~*ch.bb_color[!ch.black_to_move]);
    U64 pinned = (ray & en_rooks && BB::num_bits_flipped(ray & *ch.bb_color[ch.black_to_move]) == 1)
            ? ray & *ch.bb_color[ch.black_to_move] : 0ull;
    // East
    ray = BB::east_attacks(king, ~*ch.bb_color[!ch.black_to_move]);
    pinned |= (ray & en_rooks && BB::num_bits_flipped(ray & *ch.bb_color[ch.black_to_move]) == 1)
            ? ray & *ch.bb_color[ch.black_to_move] : 0ull;
    // South
    ray = BB::sout_attacks(king, ~*ch.bb_color[!ch.black_to_move]);
    pinned |= (ray & en_rooks && BB::num_bits_flipped(ray & *ch.bb_color[ch.black_to_move]) == 1)
            ? ray & *ch.bb_color[ch.black_to_move] : 0ull;
    // West
    ray = BB::west_attacks(king, ~*ch.bb_color[!ch.black_to_move]);
    pinned |= (ray & en_rooks && BB::num_bits_flipped(ray & *ch.bb_color[ch.black_to_move]) == 1)
            ? ray & *ch.bb_color[ch.black_to_move] : 0ull;
    // NorthEast
    ray = BB::NoEa_attacks(king, ~*ch.bb_color[!ch.black_to_move]);
    pinned |= (ray & en_bishops && BB::num_bits_flipped(ray & *ch.bb_color[ch.black_to_move]) == 1)
            ? ray & *ch.bb_color[ch.black_to_move] : 0ull;
    // NorthWest
    ray = BB::NoWe_attacks(king, ~*ch.bb_color[!ch.black_to_move]);
    pinned |= (ray & en_bishops && BB::num_bits_flipped(ray & *ch.bb_color[ch.black_to_move]) == 1)
            ? ray & *ch.bb_color[ch.black_to_move] : 0ull;
    // SouthEast
    ray = BB::SoEa_attacks(king, ~*ch.bb_color[!ch.black_to_move]);
    pinned |= (ray & en_bishops && BB::num_bits_flipped(ray & *ch.bb_color[ch.black_to_move]) == 1)
            ? ray & *ch.bb_color[ch.black_to_move] : 0ull;
    // SouthWest
    ray = BB::SoWe_attacks(king, ~*ch.bb_color[!ch.black_to_move]);
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
    U64 mask = Compass::king_attacks[find_king(!ch.black_to_move)];
    U64 op = *ch.bb_color[!ch.black_to_move];
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
        mask |= Compass::knight_attacks[63 - BB::lz_count(attackers & 0-attackers)];
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
    return 63 - BB::lz_count(ch.bb_kings & *ch.bb_color[color]);
}

/*
 * Method to get the legal moves in a position.
 * @return an array of all legal moves
 */
move* MoveGenerator::gen_moves(move (&moves)[120], bool test)
{
    init(test);

    // king
    moves[119] = 0;
    move temp[120] = {};
    gen_king_piece_moves(temp, test);
    for (int i = 0; i < temp[119]; i++)
    {
        moves[moves[119]] = temp[i];
        moves[119] += moves[119] < 118 ? 1 : 0;
    }
    temp[119] = 0;
    if (in_double_check) return moves;

    // rooks and queens
    U64 ss = (ch.bb_rooks | ch.bb_queens) & *ch.bb_color[ch.black_to_move];
    while (ss)
    {
        // x & -x masks the LS1B
        gen_rook_piece_moves(temp, 63 - BB::lz_count(ss & 0-ss), test);
        for (int i = 0; i < temp[119]; i++)
        {
            moves[moves[119]] = temp[i];
            moves[119] += moves[119] < 118 ? 1 : 0;
        }
        temp[119] = 0;
        // now clear that LS1B
        ss &= ss - 1;
    }

    // bishops and queens
    ss = (ch.bb_bishops | ch.bb_queens) & *ch.bb_color[ch.black_to_move];
    while (ss)
    {
        // x & -x masks the LS1B
        gen_bishop_piece_moves(temp, 63 - BB::lz_count(ss & 0-ss), test);
        for (int i = 0; i < temp[119]; i++)
        {
            moves[moves[119]] = temp[i];
            moves[119] += moves[119] < 118 ? 1 : 0;
        }
        temp[119] = 0;
        // now clear that LS1B
        ss &= ss - 1;
    }

    // knights
    ss = ch.bb_knights & *ch.bb_color[ch.black_to_move];
    while (ss)
    {
        // x & -x masks the LS1B
        gen_knight_piece_moves(temp, 63 - BB::lz_count(ss & 0-ss), test);
        for (int i = 0; i < temp[119]; i++)
        {
            moves[moves[119]] = temp[i];
            moves[119] += moves[119] < 118 ? 1 : 0;
        }
        temp[119] = 0;
        // now clear that LS1B
        ss &= ss - 1;
    }

    // pawns
    gen_pawn_moves(temp, test);
    for (int i = 0; i < temp[119]; i++)
    {
        moves[moves[119]] = temp[i];
        moves[119] += moves[119] < 118 ? 1 : 0;
    }
    return moves;
}

/*
 * Generates legal pawn moves
 * @return an unsorted list of pawn moves
 */
void MoveGenerator::gen_pawn_moves(move (&pawn_moves)[120], bool test)
{
    using namespace directions;
    U64 pawns = ch.bb_pawns & *ch.bb_color[ch.black_to_move];
    pawn_moves[120] = {};
    if (!pawns) return;
    U64 op = *ch.bb_color[!ch.black_to_move];

    // pawn captures
    U64 captures_east = (ch.black_to_move) ? BB::SoEa_shift_one(pawns) & (op | 1ull << ch.ep_square) : BB::NoEa_shift_one(pawns) & (op | 1ull << ch.ep_square);
    U64 captures_west = (ch.black_to_move) ? BB::SoWe_shift_one(pawns) & (op | 1ull << ch.ep_square) : BB::NoWe_shift_one(pawns) & (op | 1ull << ch.ep_square);

    // eastern captures
    while (captures_east)
    {
        // x & -x masks the LS1B
        int end_sq = 63 - BB::lz_count(captures_east & 0-captures_east);
        // now clear that LS1B
        captures_east &= captures_east - 1;
        int start_sq = end_sq - DIRS[4 + 2 * ch.black_to_move];
        if (BB::contains_square(pinned_pieces, start_sq)
                && !BB::contains_square(Compass::ray_square(find_king(ch.black_to_move), start_sq, op), end_sq))
            continue;
        if (Compass::rank_yindex(end_sq) % 7 != 0)
        {
            pawn_moves[pawn_moves[119]] = Move::build_move(start_sq, end_sq);
            pawn_moves[119]++;
        }
        else // pawn promotions
        {
            pawn_moves[pawn_moves[119]] = Move::build_move(start_sq, end_sq, ch_cst::QUEEN);
            pawn_moves[119]++;
            pawn_moves[pawn_moves[119]] = Move::build_move(start_sq, end_sq, ch_cst::ROOK);
            pawn_moves[119]++;
            pawn_moves[pawn_moves[119]] = Move::build_move(start_sq, end_sq, ch_cst::KNIGHT);
            pawn_moves[119]++;
            pawn_moves[pawn_moves[119]] = Move::build_move(start_sq, end_sq, ch_cst::BISHOP);
            pawn_moves[119]++;
        }
    }

    // western captures
    while (captures_west)
    {
        // x & -x masks the LS1B
        int end_sq = 63 - BB::lz_count(captures_west & 0-captures_west);
        // now clear that LS1B
        captures_west &= captures_west - 1;
        int start_sq = end_sq - DIRS[5 + 2 * ch.black_to_move];
        if (BB::contains_square(pinned_pieces, start_sq)
                && !BB::contains_square(Compass::ray_square(find_king(ch.black_to_move), start_sq, op), end_sq))
            continue;
        if (Compass::rank_yindex(end_sq) % 7 != 0)
        {
            pawn_moves[pawn_moves[119]] = Move::build_move(start_sq, end_sq);
            pawn_moves[119]++;
        }
        else // pawn promotions
        {
            pawn_moves[pawn_moves[119]] = Move::build_move(start_sq, end_sq, ch_cst::QUEEN);
            pawn_moves[119]++;
            pawn_moves[pawn_moves[119]] = Move::build_move(start_sq, end_sq, ch_cst::ROOK);
            pawn_moves[119]++;
            pawn_moves[pawn_moves[119]] = Move::build_move(start_sq, end_sq, ch_cst::KNIGHT);
            pawn_moves[119]++;
            pawn_moves[pawn_moves[119]] = Move::build_move(start_sq, end_sq, ch_cst::BISHOP);
            pawn_moves[119]++;
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
        int end_sq = 63 - BB::lz_count(pawn_advances & 0-pawn_advances);
         // now clear that LS1B
        pawn_advances &= pawn_advances - 1;
        int start_sq = end_sq - PAWN_DIR[ch.black_to_move];
        if (BB::contains_square(pawns, start_sq)
                && (!BB::contains_square(pinned_pieces, start_sq)
                    || Compass::file_xindex(find_king(ch.black_to_move)) == Compass::file_xindex(end_sq)))
        {
            if (Compass::rank_yindex(end_sq) % 7 != 0)
            {
                pawn_moves[pawn_moves[119]] = Move::build_move(start_sq, end_sq);
                pawn_moves[119]++;
            }
            else // pawn promotions
            {
                pawn_moves[pawn_moves[119]] = Move::build_move(start_sq, end_sq, ch_cst::QUEEN);
                pawn_moves[119]++;
                pawn_moves[pawn_moves[119]] = Move::build_move(start_sq, end_sq, ch_cst::ROOK);
                pawn_moves[119]++;
                pawn_moves[pawn_moves[119]] = Move::build_move(start_sq, end_sq, ch_cst::KNIGHT);
                pawn_moves[119]++;
                pawn_moves[pawn_moves[119]] = Move::build_move(start_sq, end_sq, ch_cst::BISHOP);
                pawn_moves[119]++;
            }
        }
        // double advances
        else if (BB::contains_square(pawns, end_sq - 2 * PAWN_DIR[ch.black_to_move]) &&
            (!BB::contains_square(pinned_pieces, end_sq - 2 * PAWN_DIR[ch.black_to_move])
            || Compass::file_xindex(find_king(ch.black_to_move)) == Compass::file_xindex(end_sq)))
        {
            pawn_moves[pawn_moves[119]] = Move::build_move(end_sq - 2 * PAWN_DIR[ch.black_to_move], end_sq);
            pawn_moves[119]++;
        }
    }

    if (!in_check)
        return;
    move legal_moves[120] = {};
    for (int i = 0; i < pawn_moves[119]; i++)
        if (BB::contains_square(check_ray, Move::end(pawn_moves[i]))
            || (Move::end(pawn_moves[i]) == ch.ep_square && check_ray & 1ull << (ch.ep_square + directions::PAWN_DIR[!ch.black_to_move])))
        {
            legal_moves[legal_moves[119]] = pawn_moves[i];
            legal_moves[119]++;
        }
    for (int i = 0; i < legal_moves[119]; i++)
        pawn_moves[i] = legal_moves[i];
    pawn_moves[119] = legal_moves[119];
}

void MoveGenerator::gen_knight_piece_moves(move (&knight_moves)[120], int start, bool test)
{
    U64 ts = Compass::knight_attacks[start] & ~*ch.bb_color[ch.black_to_move];
    if (BB::contains_square(pinned_pieces, start))
        return;

    while (ts)
    {
        // x & -x masks the LS1B
        knight_moves[knight_moves[119]] = Move::build_move(start, 63 - BB::lz_count(ts & 0-ts));
        knight_moves[119]++;
        // now clear that LS1B
        ts &= ts - 1;
    }

    if (!in_check)
        return;
    move legal_moves[120] = {};
    for (int i = 0; i < knight_moves[119]; i++)
        if (check_ray & 1ull << Move::end(knight_moves[i]))
        {
            legal_moves[legal_moves[119]] = knight_moves[i];
            legal_moves[119]++;
        }
    for (int i = 0; i < legal_moves[119]; i++)
        knight_moves[i] = legal_moves[i];
    knight_moves[119] = legal_moves[119];
}

void MoveGenerator::gen_bishop_piece_moves(move (&bishop_moves)[120], int start, bool test)
{
    bishop_moves[120] = {};
    U64 ss = 1ull << start;
    U64 ts = BB::NoEa_attacks(ss, ~ch.bb_occ);
    ts |= BB::NoWe_attacks(ss, ~ch.bb_occ);
    ts |= BB::SoEa_attacks(ss, ~ch.bb_occ);
    ts |= BB::SoWe_attacks(ss, ~ch.bb_occ);
    ts &= *ch.bb_color[!ch.black_to_move] | ~ch.bb_occ;

    while (ts)
    {
        // x & -x masks the LS1B
        if (~pinned_pieces & ss || Compass::ray_square(find_king(ch.black_to_move), start) & ts & 0-ts)
        {
            bishop_moves[bishop_moves[119]] = Move::build_move(start, 63 - BB::lz_count(ts & 0-ts));
            bishop_moves[119]++;
        }
        // now clear that LS1B
        ts &= ts - 1;
    }

    if (!in_check)
        return;
    move legal_moves[120] = {};
    for (int i = 0; i < bishop_moves[119]; i++)
        if (BB::contains_square(check_ray, Move::end(bishop_moves[i])))
        {
            legal_moves[legal_moves[119]] = bishop_moves[i];
            legal_moves[119]++;
        }
    for (int i = 0; i < legal_moves[119]; i++)
        bishop_moves[i] = legal_moves[i];
    bishop_moves[119] = legal_moves[119];
}

void MoveGenerator::gen_rook_piece_moves(move (&rook_moves)[120], int start, bool test)
{
    U64 ss = 1ull << start;
    U64 ts = BB::nort_attacks(ss, ~ch.bb_occ);
    ts |= BB::sout_attacks(ss, ~ch.bb_occ);
    ts |= BB::east_attacks(ss, ~ch.bb_occ);
    ts |= BB::west_attacks(ss, ~ch.bb_occ);
    ts &= *ch.bb_color[!ch.black_to_move] | ~ch.bb_occ;

    // ignore pinned pieces
    while (ts)
    {
        // x & -x masks the LS1B
        if (~pinned_pieces & ss || Compass::ray_square(find_king(ch.black_to_move), start) & ts & 0-ts)
        {
            rook_moves[rook_moves[119]] = Move::build_move(start, 63 - BB::lz_count(ts & 0-ts));
            rook_moves[119]++;
        }
        // now clear that LS1B
        ts &= ts - 1;
    }

    if (!in_check)
        return;
    move legal_moves[120] = {};
    for (int i = 0; i < rook_moves[119]; i++)
        if (BB::contains_square(check_ray, Move::end(rook_moves[i])))
        {
            legal_moves[legal_moves[119]] = rook_moves[i];
            legal_moves[119]++;
        }
    for (int i = 0; i < legal_moves[119]; i++)
        rook_moves[i] = legal_moves[i];
    rook_moves[119] = legal_moves[119];
}

void MoveGenerator::gen_king_piece_moves(move (&king_moves)[120], bool test)
{
    int king_sq = find_king(ch.black_to_move);
    U64 ts = Compass::king_attacks[king_sq]
           & ~*ch.bb_color[ch.black_to_move]
           & ~op_attack_mask;

    // normal moves
    while (ts)
    {
        // x & -x masks the LS1B
        king_moves[king_moves[119]] = Move::build_move(king_sq, 63 - BB::lz_count(ts & 0-ts));
        king_moves[119]++;
        // now clear that LS1B
        ts &= ts - 1;
    }

    // castling
    // wh:QuKi bl:QuKi
    // queenside castle
    if (ch.castle_rights & 2 << 2 * ch.black_to_move && !in_check
        && !BB::contains_square(ch.bb_occ | op_attack_mask, king_sq - 1)
        && !BB::contains_square(ch.bb_occ | op_attack_mask, king_sq - 2)
        && !BB::contains_square(ch.bb_occ, king_sq - 3)
        && BB::contains_square(ch.bb_rooks & *ch.bb_color[ch.black_to_move], king_sq - 4))
    {
        king_moves[king_moves[119]] = Move::build_move(king_sq, king_sq - 2);
        king_moves[119]++;
    }
    // kingside castle
    if (ch.castle_rights & 1 << 2 * ch.black_to_move && !in_check
        && !BB::contains_square(ch.bb_occ | op_attack_mask, king_sq + 1)
        && !BB::contains_square(ch.bb_occ | op_attack_mask, king_sq + 2)
        && BB::contains_square(ch.bb_rooks & *ch.bb_color[ch.black_to_move], king_sq + 3))
    {
        king_moves[king_moves[119]] = Move::build_move(king_sq, king_sq + 2);
        king_moves[119]++;
    }
}

/*
 * Method to write the SAN string of a move in the current position.
 * @param mv the Move to write
 * @returns a string of the SAN of the move
 */
std::string MoveGenerator::move_san(Chess ch, move mv)
{
    MoveGenerator san_gen(ch);
    move moves[120] = {};
    san_gen.gen_moves(moves);
    std::string san = "";
    int start = Move::start(mv), end = Move::end(mv);
    int piece = ch.piece_at(start);

    // check for ambiguity
    for (int i = 0; i < moves[119]; i++)
    {
        move m2 = moves[i];
        if (Move::start(m2) == start || Move::end(m2) != end || ch.piece_at(Move::start(m2)) != piece)
            continue;
        if (Compass::file_xindex(Move::start(m2)) != Compass::file_xindex(start))
            san = ch_cst::string_from_square[start].substr(0, 1) + san;
        if (Compass::rank_yindex(Move::start(m2)) != Compass::rank_yindex(start))
            san += ch_cst::string_from_square[start].substr(1, 1);
    }

    if (piece != ch_cst::PAWN)
        san = ch_cst::piece_char[piece] + san;

    // captures
    if (BB::contains_square(ch.bb_occ, end) || piece == ch_cst::PAWN && end == ch.ep_square)
    {
        if (piece == ch_cst::PAWN)
            san = ch_cst::string_from_square[start].substr(0, 1);
        san += "x";
    }
    san += ch_cst::string_from_square[end];

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
    ch.make_move(mv);
    san_gen.init(false);
    if (san_gen.in_check)
        san += "+";
    ch.unmake_move(1);

    return san;
}

