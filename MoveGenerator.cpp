#include "MoveGenerator.h"

/*
 * Default constructor.
 */
MoveGenerator::MoveGenerator(Chess &ch)
{
    chess = ch;
    init();
}

/*
 * Passes a new chess position to the move generator
 * @param ch the chess position
 */
void MoveGenerator::set_chess(Chess &ch)
{
    chess = ch;
    init();
}

/*
 * Run some setup functions to prepare for move generation
 */
void MoveGenerator::init()
{
    gen_op_attack_mask();
    find_pins();
    check_exists();
}

/*
 * Method to determine whether a game has ended.
 * Must call AFTER init(): we use gen_moves().size() to determine state.
 * @return true if stalemate, checkmate
 * @return false if any legal moves exist
 */
bool MoveGenerator::is_game_over()
{
    return gen_moves().size() == 0;
}

void MoveGenerator::check_exists()
{
    using namespace directions;
    U64 bb_king = chess.bb_kings & *chess.bb_by_color[chess.aci];
    in_check = false;
    in_double_check = false;
    check_ray = 0;
    if (!(bb_king & op_attack_mask))
        return;
    U64 op = *chess.bb_by_color[1-chess.aci];
    int ksq = find_king(chess.aci);

    // knights
    U64 attackers = chess.bb_knights & op & Compass::knight_attacks[ksq];
    if (attackers)
    {
        check_ray |= 1ull << (63 - Bitboard::leading_zeros(attackers));
        check_method();
    }

    // pawns
    attackers = Bitboard::gen_shift(bb_king & Bitboard::NOT_H_FILE, DIRS[4 + 2 * (chess.aci)]);
    if (attackers & op & chess.bb_pawns)
    {
        check_ray |= 1ull << (63 - Bitboard::leading_zeros(attackers));
        check_method();
    }
    attackers = Bitboard::gen_shift(bb_king & Bitboard::NOT_A_FILE, DIRS[5 + 2 * (chess.aci)]);
    if (attackers & op & chess.bb_pawns)
    {
        check_ray |= 1ull << (63 - Bitboard::leading_zeros(attackers));
        check_method();
    }


    // bishops
    attackers = Bitboard::NoEa_attacks(bb_king, ~chess.bb_occ);
    if (attackers & (chess.bb_queens | chess.bb_bishops) & op)
    {
        check_ray = attackers;
        check_method();
    }
    attackers = Bitboard::NoWe_attacks(bb_king, ~chess.bb_occ);
    if (!in_double_check && attackers & (chess.bb_queens | chess.bb_bishops) & op)
    {
        check_ray = attackers;
        check_method();
    }
    attackers = Bitboard::SoEa_attacks(bb_king, ~chess.bb_occ);
    if (!in_double_check && attackers & (chess.bb_queens | chess.bb_bishops) & op)
    {
        check_ray = attackers;
        check_method();
    }
    attackers = Bitboard::SoWe_attacks(bb_king, ~chess.bb_occ);
    if (!in_double_check && attackers & (chess.bb_queens | chess.bb_bishops) & op)
    {
        check_ray = attackers;
        check_method();
    }

    // rooks
    attackers = Bitboard::nort_attacks(bb_king, ~chess.bb_occ);
    if (!in_double_check && attackers & (chess.bb_queens | chess.bb_rooks) & op)
    {
        check_ray = attackers;
        check_method();
    }
    attackers = Bitboard::sout_attacks(bb_king, ~chess.bb_occ);
    if (!in_double_check && attackers & (chess.bb_queens | chess.bb_rooks) & op)
    {
        check_ray = attackers;
        check_method();
    }
    attackers = Bitboard::east_attacks(bb_king, ~chess.bb_occ);
    if (!in_double_check && attackers & (chess.bb_queens | chess.bb_bishops) & op)
    {
        check_ray = attackers;
        check_method();
    }
    attackers = Bitboard::west_attacks(bb_king, ~chess.bb_occ);
    if (!in_double_check && attackers & (chess.bb_queens | chess.bb_bishops) & op)
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
void MoveGenerator::find_pins()
{
    using namespace directions;
    // check each ray that contains the king square
    // look for rays that pass an allied piece,
    // then a relevent sliding piece
    pinned_pieces = 0;
    pin_ray_moves = 0;
    // find the active king
    int ksq = find_king(chess.aci);
    // check each ray for a pin sequence
    for (int i = Compass::get_dir_start_index(ch_cst::QUEEN); i < Compass::get_dir_end_index(ch_cst::QUEEN); i++)
    {
        U64 pin_ray = Compass::build_ray(ksq+DIRS[i], i);
        // determine the type of piece that could pin along this ray
        int type = i < 4 ? ch_cst::ROOK : ch_cst::BISHOP;
        U64 pinning_piece = pin_ray & (*chess.bb_by_piece[type] | chess.bb_queens) & *chess.bb_by_color[1 - chess.aci];
        if (!pinning_piece)
            continue;
        for (int step = 2; step <= Compass::edge_distance64x8[ksq][i]; step++)
        {
            if (!Bitboard::contains_square(pinning_piece, ksq + step * DIRS[i]))
                continue;
            int points[2] = { ksq, ksq + step * DIRS[i] };
            pin_ray = Compass::build_ray(points) & *chess.bb_by_color[chess.aci];
            if (Bitboard::num_bits_flipped(pin_ray) == 1)
            {
                int pinned_piece_sq;
                for (int shift = 0; chess.bb_occ >> shift > 0; shift++)
                {
                    if (pin_ray >> shift == 1)
                    {
                        pinned_piece_sq = shift;
                        pinned_pieces |= 1ull << pinned_piece_sq;
                        int points[2] = { pinned_piece_sq, ksq + step * DIRS[i] };
                        pin_ray_moves |= *chess.bb_by_color[chess.aci] & Compass::build_ray(points);
                        break;
                    }
                }
            }
            break;
        }
    }
    // if (pinned_pieces)
    // {
    //     std::cout << "Pins exist!" << std::endl;
    //     Bitboard::print_binary_string(Bitboard::build_binary_string(pinned_pieces));
    //     Bitboard::print_binary_string(Bitboard::build_binary_string(pin_ray_moves));
    // }
}

/*
 * Method to get the opponents' attack mask
 * @return the bitboard of squares attacked by opponent's pieces
 */
void MoveGenerator::gen_op_attack_mask()
{
    op_attack_mask = Compass::king_attacks[find_king(1-chess.aci)];
    U64 op = *chess.bb_by_color[1-chess.aci];
    // pawn attacks
    // white, shift north
    U64 pattacksWest = Bitboard::NoWe_shift_one(chess.bb_pawns & op);
    U64 pattacksEast = Bitboard::NoEa_shift_one(chess.bb_pawns & op);
    if (!chess.aci)
    { // black, shift south
        pattacksWest = Bitboard::SoWe_shift_one(chess.bb_pawns & op);
        pattacksEast = Bitboard::SoEa_shift_one(chess.bb_pawns & op);
    }
    op_attack_mask |= pattacksEast;
    op_attack_mask |= pattacksWest;
    U64 sliders = (chess.bb_rooks | chess.bb_queens) & op;
    op_attack_mask |= Bitboard::nort_attacks(sliders, ~chess.bb_occ);
    op_attack_mask |= Bitboard::sout_attacks(sliders, ~chess.bb_occ);
    op_attack_mask |= Bitboard::east_attacks(sliders, ~chess.bb_occ);
    op_attack_mask |= Bitboard::west_attacks(sliders, ~chess.bb_occ);
    sliders = (chess.bb_bishops | chess.bb_queens) & op;
    op_attack_mask |= Bitboard::NoEa_attacks(sliders, ~chess.bb_occ);
    op_attack_mask |= Bitboard::NoWe_attacks(sliders, ~chess.bb_occ);
    op_attack_mask |= Bitboard::SoEa_attacks(sliders, ~chess.bb_occ);
    op_attack_mask |= Bitboard::SoWe_attacks(sliders, ~chess.bb_occ);
    for (int sq = 0; chess.bb_knights & op >> sq; sq++)
        if (Bitboard::contains_square(chess.bb_knights & op, sq))
            op_attack_mask |= Compass::knight_attacks[sq];
}

/*
 * Method to find a given king.
 * @param color the color index of the king to search for
 * @return the square index of the king (0-63)
 */
int MoveGenerator::find_king(int color)
{
    return 63 - Bitboard::leading_zeros(chess.bb_kings & *chess.bb_by_color[color]);
}

/*
 * Method to get the legal moves in a position.
 * Remeber to call init() first.
 * @return an unsorted vector of moves
 */
std::vector<Move> MoveGenerator::gen_moves()
{
    std::vector<Move> moves;
    moves = gen_pawn_moves();
    for (int sq = 0; *chess.bb_by_color[chess.aci] >> sq; sq++)
    {
        if (!Bitboard::contains_square(*chess.bb_by_color[chess.aci], sq))
            continue;
        if (Bitboard::contains_square(*chess.bb_by_piece[ch_cst::KNIGHT], sq))
        {
            for (Move m : gen_knight_piece_moves(sq))
                moves.push_back(m);
        }
        else if (Bitboard::contains_square(*chess.bb_by_piece[ch_cst::KING], sq))
        {
            for (Move m : gen_king_piece_moves(sq))
                moves.push_back(m);
        }
        else if (Bitboard::contains_square(*chess.bb_by_piece[ch_cst::BISHOP], sq))
        {
            
            for (Move m : gen_bishop_piece_moves(sq))
                moves.push_back(m);
        }
        else if (Bitboard::contains_square(*chess.bb_by_piece[ch_cst::ROOK], sq))
        {
            for (Move m : gen_rook_piece_moves(sq))
                moves.push_back(m);
        }
        else if (Bitboard::contains_square(*chess.bb_by_piece[ch_cst::QUEEN], sq))
        {
            for (Move m : gen_bishop_piece_moves(sq))
                moves.push_back(m);
            for (Move m : gen_rook_piece_moves(sq))
                moves.push_back(m);
        }
    }
    return moves;
}

/*
 * Generates legal pawn moves
 * @return an unsorted list of pawn moves
 */
std::vector<Move> MoveGenerator::gen_pawn_moves()
{
    using namespace directions;
    std::vector<Move> pawn_moves;
    U64 pawns = chess.bb_pawns & *chess.bb_by_color[chess.aci];
    if (!pawns || in_double_check)
        return pawn_moves;
    U64 op = *chess.bb_by_color[1 - chess.aci];
    // pawn captures
    // white to move, shift left
    U64 capturesWest = Bitboard::NoWe_shift_one(pawns) & op;
    U64 capturesEast = Bitboard::NoEa_shift_one(pawns) & op;
    if (chess.aci)
    {
        // black to move, shift right
        capturesWest = Bitboard::SoWe_shift_one(pawns) & op;
        capturesEast = Bitboard::SoEa_shift_one(pawns) & op;
    }
    for (int sq = (chess.aci) ? 0 : 16; capturesEast >> sq; sq++)
    {
        if (!Bitboard::contains_square(capturesEast, sq))
            continue;
        if (Bitboard::contains_square(pinned_pieces, sq - DIRS[4+2*chess.aci])
            && !Bitboard::contains_square(pin_ray_moves, sq))
            continue;
        if (Compass::rank_yindex(sq) % 7 != 0)
            pawn_moves.push_back(Move(sq - DIRS[4+2*chess.aci], sq));
        else // pawn promotions
        {
            pawn_moves.push_back(Move(sq - DIRS[4+2*chess.aci], sq, ch_cst::QUEEN));
            pawn_moves.push_back(Move(sq - DIRS[4+2*chess.aci], sq, ch_cst::ROOK));
            pawn_moves.push_back(Move(sq - DIRS[4+2*chess.aci], sq, ch_cst::BISHOP));
            pawn_moves.push_back(Move(sq - DIRS[4+2*chess.aci], sq, ch_cst::KNIGHT));
        }
    }
    for (int sq = (chess.aci) ? 0 : 16; capturesWest >> sq > 0; sq++)
    {
        if (!Bitboard::contains_square(capturesWest, sq))
            continue;
        if (Bitboard::contains_square(pinned_pieces, sq - DIRS[5+2*chess.aci])
            && !Bitboard::contains_square(pin_ray_moves, sq))
            continue;
        if (Compass::rank_yindex(sq) % 7 != 0)
            pawn_moves.push_back(Move(sq - DIRS[5+2*chess.aci], sq));
        else // pawn promotions
        {
            pawn_moves.push_back(Move(sq - DIRS[5+2*chess.aci], sq, ch_cst::QUEEN));
            pawn_moves.push_back(Move(sq - DIRS[5+2*chess.aci], sq, ch_cst::ROOK));
            pawn_moves.push_back(Move(sq - DIRS[5+2*chess.aci], sq, ch_cst::BISHOP));
            pawn_moves.push_back(Move(sq - DIRS[5+2*chess.aci], sq, ch_cst::KNIGHT));
        }
    }
    // pawn moves
    if (!chess.aci)
    {
        // white moves
        pawns = (pawns << 8) & ~chess.bb_occ;
        pawns |= ((pawns & 255ull << 16) << 8) & ~chess.bb_occ;
    }
    else
    {
        // black moves
        pawns = (pawns >> 8) & ~chess.bb_occ;
        pawns |= ((pawns & 255ull << 40) >> 8) & ~chess.bb_occ;
    }
    for (int sq = (chess.aci) ? 0 : 16; pawns >> sq; sq++)
    {
        if (!Bitboard::contains_square(pawns, sq))
            continue;
        if (Bitboard::contains_square(chess.bb_pawns & *chess.bb_by_color[chess.aci], sq - PAWN_DIR[chess.aci])
        && (!Bitboard::contains_square(pinned_pieces, sq - PAWN_DIR[chess.aci])
            || Compass::file_xindex(find_king(chess.aci)) == Compass::file_xindex(sq)))
        {
            if (Compass::rank_yindex(sq) % 7 != 0)
                pawn_moves.push_back(Move(sq - PAWN_DIR[chess.aci], sq));
            else // pawn promotions
            {
                pawn_moves.push_back(Move(sq - PAWN_DIR[chess.aci], sq, ch_cst::QUEEN));
                pawn_moves.push_back(Move(sq - PAWN_DIR[chess.aci], sq, ch_cst::ROOK));
                pawn_moves.push_back(Move(sq - PAWN_DIR[chess.aci], sq, ch_cst::BISHOP));
                pawn_moves.push_back(Move(sq - PAWN_DIR[chess.aci], sq, ch_cst::KNIGHT));
            }
        }
        else if (!Bitboard::contains_square(pinned_pieces, sq - 2 * PAWN_DIR[chess.aci])
            || Compass::file_xindex(find_king(chess.aci)) == Compass::file_xindex(sq))
            pawn_moves.push_back(Move(sq - 2 * PAWN_DIR[chess.aci], sq));
    }
    if (!in_check)
        return pawn_moves;
    std::vector<Move> legal_moves;
    for (Move m : pawn_moves)
    {
        if (check_ray & 1ull << m.end()
            || (m.end() == chess.ep_square && check_ray & 1ull << (chess.ep_square + directions::PAWN_DIR[1-chess.aci])))
            legal_moves.push_back(m);
    }
    return legal_moves;
}

std::vector<Move> MoveGenerator::gen_knight_piece_moves(int sq)
{
    std::vector<Move> knight_moves;
    U64 moves = Compass::knight_attacks[sq] & ~*chess.bb_by_color[chess.aci];
    if (!moves || in_double_check)
        return knight_moves;
    int end_index = std::min(64, sq + 18);
    for (int end = std::max(0, sq - 17); end < end_index; end++)
    {
        if (!Bitboard::contains_square(moves, end))
            continue;
        if (Bitboard::contains_square(pinned_pieces, sq)
        && !Bitboard::contains_square(pin_ray_moves, end))
            continue;
        knight_moves.push_back(Move(sq, end));
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

std::vector<Move> MoveGenerator::gen_bishop_piece_moves(int sq)
{
    std::vector<Move> bishop_moves;
    if (in_double_check)
        return bishop_moves;
    U64 attacks = Bitboard::NoEa_attacks(1ull << sq, ~chess.bb_occ);
    attacks |= Bitboard::NoWe_attacks(1ull << sq, ~chess.bb_occ);
    attacks |= Bitboard::SoEa_attacks(1ull << sq, ~chess.bb_occ);
    attacks |= Bitboard::SoWe_attacks(1ull << sq, ~chess.bb_occ);
    attacks &= *chess.bb_by_color[1-chess.aci] | ~chess.bb_occ;
    for (int target_sq = 0; attacks >> target_sq; target_sq++)
    {
        if (!Bitboard::contains_square(attacks, target_sq)
            || (pinned_pieces & 1ull << sq && !(pin_ray_moves & 1ull << target_sq)))
            continue;
        bishop_moves.push_back(Move(sq, target_sq));
    }
    if (!in_check)
        return bishop_moves;
    std::vector<Move> legal_moves;
    for (Move m : bishop_moves)
    {
        if (check_ray & 1ull << m.end())
            legal_moves.push_back(m);
    }
    return legal_moves;
}

std::vector<Move> MoveGenerator::gen_rook_piece_moves(int sq)
{
    std::vector<Move> rook_moves;
    if (in_double_check)
        return rook_moves;
    U64 attacks = Bitboard::nort_attacks(1ull << sq, ~chess.bb_occ);
    attacks |= Bitboard::sout_attacks(1ull << sq, ~chess.bb_occ);
    attacks |= Bitboard::east_attacks(1ull << sq, ~chess.bb_occ);
    attacks |= Bitboard::west_attacks(1ull << sq, ~chess.bb_occ);
    attacks &= *chess.bb_by_color[1-chess.aci] | ~chess.bb_occ;
    for (int target_sq = 0; attacks >> target_sq; target_sq++)
    {
        if (!Bitboard::contains_square(attacks, target_sq)
            || (pinned_pieces & 1ull << sq && !(pin_ray_moves & 1ull << target_sq)))
            continue;
        rook_moves.push_back(Move(sq, target_sq));
    }
    if (!in_check)
        return rook_moves;
    std::vector<Move> legal_moves;
    for (Move m : rook_moves)
    {
        if (check_ray & 1ull << m.end())
            legal_moves.push_back(m);
    }
    return legal_moves;
}

std::vector<Move> MoveGenerator::gen_king_piece_moves(int sq)
{
    std::vector<Move> king_moves;
    U64 moves = Compass::king_attacks[sq] & ~*chess.bb_by_color[chess.aci] & ~op_attack_mask;
    if (!moves)
        return king_moves;
    // normal moves
    int end = std::min(64, sq + 10);
    for (int endsq = std::max(0, sq - 9); endsq < end; endsq++)
    {
        if (!Bitboard::contains_square(moves, endsq))
            continue;
        king_moves.push_back(Move(sq, endsq));
    }
    // castling
    // wh:QuKi bl:QuKi
    // queenside castle
    if (chess.castle_rights & 2 << 2 * chess.aci
        && !Bitboard::contains_square(chess.bb_occ, sq - 1)
        && !Bitboard::contains_square(chess.bb_occ, sq - 2)
        && !Bitboard::contains_square(chess.bb_occ, sq - 3)
        && Bitboard::contains_square(chess.bb_rooks & *chess.bb_by_color[chess.aci], sq - 4))
        king_moves.push_back(Move(sq, sq - 2));
    // kingside castle
    if (chess.castle_rights & 1 << 2 * chess.aci
        && !Bitboard::contains_square(chess.bb_occ, sq + 1)
        && !Bitboard::contains_square(chess.bb_occ, sq + 2)
        && Bitboard::contains_square(chess.bb_rooks & *chess.bb_by_color[chess.aci], sq + 3))
        king_moves.push_back(Move(sq, sq + 2));
    return king_moves;
}
