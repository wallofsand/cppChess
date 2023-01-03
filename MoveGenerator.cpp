#include "MoveGenerator.h"

using namespace directions;

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

bool MoveGenerator::is_game_over()
{
    if (gen_moves().size() == 0)
    {
        return true;
    }
    return false;
}

void MoveGenerator::check_exists()
{
    U64 op = *chess.bb_by_color[1-chess.aci];
    int ksq = find_king(chess.aci);
    in_check = false;
    in_double_check = false;
    if (!chess.aci && Bitboard::contains_square((chess.bb_pawns & op & Bitboard::NOT_H_FILE), ksq + NORTHWEST))
        check_method(ksq + NORTHWEST);
    if (!chess.aci && Bitboard::contains_square((chess.bb_pawns & op & Bitboard::NOT_A_FILE), ksq + NORTHEAST))
        check_method(ksq + NORTHEAST);
    if (chess.aci && Bitboard::contains_square((chess.bb_pawns & op & Bitboard::NOT_H_FILE), ksq + SOUTHWEST))
        check_method(ksq + SOUTHWEST);
    if (chess.aci && Bitboard::contains_square((chess.bb_pawns & op & Bitboard::NOT_A_FILE), ksq + SOUTHEAST))
        check_method(ksq + SOUTHEAST);
    for (Move m : gen_knight_piece_moves(ksq))
        if (Bitboard::contains_square(chess.bb_knights & op, m.end()))
            check_method(m.end());
    for (Move m : gen_bishop_piece_moves(ksq))
        if (Bitboard::contains_square(chess.bb_queens & chess.bb_bishops & op, m.end()))
            check_method(m.end());
    for (Move m : gen_rook_piece_moves(ksq))
        if (Bitboard::contains_square(chess.bb_queens & chess.bb_rooks & op, m.end()))
            check_method(m.end());
    // if there is single check, find the ray that the check exists upon
    if (in_check && !in_double_check)
    {
        if (Bitboard::contains_square(chess.bb_knights, check_square)
            || Bitboard::contains_square(chess.bb_pawns, check_square))
        {
            check_ray = 1L << check_square;
        } else {
            int squares[2] = { ksq, check_square };
            check_ray = Compass::build_ray(squares);
        }
    }
}

void MoveGenerator::check_method(int sq)
{
    if (in_check)
    {
        in_double_check = true;
        return;
    }
    in_check = true;
    check_square = sq;
}

// setup method to get pins in a position
// modifies a bitboard of squares containing pinning pieces
void MoveGenerator::find_pins()
{
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
        U64 pinning_piece = pin_ray & (*chess.bb_by_piece[type] | chess.bb_queens) & *chess.bb_by_color[~chess.aci & 1];
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
                for (int shift = 0; shift < 64; shift++)
                {
                    if (pin_ray >> shift == 1)
                    {
                        pinned_piece_sq = shift;
                        pinned_pieces |= 1ull << pinned_piece_sq;
                        int points[2] = { pinned_piece_sq, ksq + step * DIRS[i] };
                        pin_ray_moves |= Compass::build_ray(points);
                        break;
                    }
                }
                // pinned_pieces |= pin_ray;
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

U64 MoveGenerator::gen_op_attack_mask()
{
    U64 op_attack_mask = Compass::king_attacks[find_king(1-chess.aci)];
    U64 op = *chess.bb_by_color[1-chess.aci];
    // pawn attacks
    // white, shift left
    U64 pattacksWest = (chess.bb_pawns & op & Bitboard::NOT_A_FILE) << NORTHWEST;
    U64 pattacksEast = (chess.bb_pawns & op & Bitboard::NOT_H_FILE) << NORTHEAST;
    if (1-chess.aci)
    { // black, shift right
        pattacksWest = (chess.bb_pawns & op & Bitboard::NOT_A_FILE) >> NORTHEAST;
        pattacksEast = (chess.bb_pawns & op & Bitboard::NOT_H_FILE) >> NORTHWEST;
    }
    op_attack_mask |= pattacksEast;
    op_attack_mask |= pattacksWest;
    for (int sq = 0; sq < 64; sq++)
    {
        if (!Bitboard::contains_square(op, sq))
            continue;
        if (Bitboard::contains_square(chess.bb_knights, sq))
            op_attack_mask |= Compass::knight_attacks[sq];
        if (Bitboard::contains_square(chess.bb_bishops | chess.bb_queens, sq))
            for (int i = 4; i < 8; i++)
            {
                for (int step = 1; step <= Compass::edge_distance64x8[sq][i]; step++)
                {
                    op_attack_mask |= 1ull << sq + step * DIRS[i];
                    if (Bitboard::contains_square(chess.bb_occ, sq + step * DIRS[i]))
                        break;
                }
            }
        if (Bitboard::contains_square(chess.bb_rooks | chess.bb_queens, sq))
            for (int i = 0; i < 4; i++)
            {
                for (int step = 1; step <= Compass::edge_distance64x8[sq][i]; step++)
                {
                    op_attack_mask |= 1ull << sq + step * DIRS[i];
                    if (Bitboard::contains_square(chess.bb_occ, sq + step * DIRS[i]))
                        break;
                }
            }
    }
    return op_attack_mask;
}

int MoveGenerator::find_king(int color)
{
    U64 king = chess.bb_kings & *chess.bb_by_color[color];
    return 63 - Bitboard::leading_zeros(king);
}

std::vector<Move> MoveGenerator::gen_moves()
{
    init();
    std::vector<Move> moves;
    moves = gen_pawn_moves();
    for (int sq = 0; sq < 64; sq++)
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

std::vector<Move> MoveGenerator::gen_pawn_moves()
{
    std::vector<Move> pawn_moves;
    U64 pawns = chess.bb_pawns & *chess.bb_by_color[chess.aci];
    if (!pawns)
        return pawn_moves;
    U64 op = *chess.bb_by_color[1 - chess.aci];
    // pawn captures
    // white to move, shift left
    U64 capturesWest = (pawns & ~op & Bitboard::NOT_A_FILE) << NORTHWEST & op;
    U64 capturesEast = (pawns & ~op & Bitboard::NOT_H_FILE) << NORTHEAST & op;
    if (chess.aci)
    { // black to move, shift right
        capturesWest = (pawns & ~op & Bitboard::NOT_A_FILE) >> NORTHWEST & op;
        capturesEast = (pawns & ~op & Bitboard::NOT_H_FILE) >> NORTHEAST & op;
    }
    if (capturesEast)
    {
        for (int sq = 0; capturesEast >> sq; sq++)
        {
            if (!Bitboard::contains_square(capturesEast, sq))
                continue;
            if (Bitboard::contains_square(pinned_pieces, sq -DIRS[4+2*chess.aci])
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
    }
    if (capturesWest)
    {
        for (int sq = 0; capturesWest >> sq; sq++)
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
    }
    /// pawn moves
    if (chess.aci == ch_cst::WHITE_INDEX)
    {
        pawns = pawns << 8 & ~op;
        pawns |= (pawns & 255ull << 16) << 8 & ~op;
    }
    else
    {
        pawns = pawns >> 8 & ~op;
        pawns |= (pawns & 255ull << 40) >> 8 & ~op;
    }
    for (int sq = 0; sq < 64; sq++)
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
        if (m.end() == check_square || Bitboard::contains_square(check_ray, m.end()) ||
        (m.end() == chess.ep_square && check_square == chess.ep_square + directions::PAWN_DIR[1-chess.aci]))
            legal_moves.push_back(m);
    }
    return legal_moves;
}

std::vector<Move> MoveGenerator::gen_knight_piece_moves(int sq)
{
    std::vector<Move> knight_moves;
    U64 moves = Compass::knight_attacks[sq] & ~*chess.bb_by_color[chess.aci];
    for (int end = std::max(0, sq - 17); end < std::min(64, sq + 18); end++)
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
        if (m.end() == check_square || Bitboard::contains_square(check_ray, m.end()))
            legal_moves.push_back(m);
    }
    return legal_moves;
}

std::vector<Move> MoveGenerator::gen_bishop_piece_moves(int sq)
{
    std::vector<Move> bishop_moves;
    for (int i = Compass::get_dir_start_index(ch_cst::BISHOP); i < Compass::get_dir_end_index(ch_cst::BISHOP); i++)
    {
        for (int step = 1; step <= Compass::edge_distance64x8[sq][i]; step++)
        {
            if (Bitboard::contains_square(*chess.bb_by_color[chess.aci], sq+step*DIRS[i]))
                break;
            if (Bitboard::contains_square(pinned_pieces, sq)
            && !Bitboard::contains_square(pin_ray_moves, sq+step*DIRS[i]))
                continue;
            bishop_moves.push_back(Move(sq, sq+step*DIRS[i]));
            if (Bitboard::contains_square(*chess.bb_by_color[1-chess.aci], sq+step*DIRS[i]))
                break;
        }
    }
    if (!in_check)
        return bishop_moves;
    std::vector<Move> legal_moves;
    for (Move m : bishop_moves)
    {
        if (m.end() == check_square || Bitboard::contains_square(check_ray, m.end()))
            legal_moves.push_back(m);
    }
    return legal_moves;
}

std::vector<Move> MoveGenerator::gen_rook_piece_moves(int sq)
{
    std::vector<Move> rook_moves;
    // uint8_t row = (chess.bb_occ >> Compass::rank_yindex(sq) * 8) & 255;
    // uint8_t col = (chess.bb_occ_rot >> Compass::rank_yindex(sq))
    // Compass::rook_rows256x8[row][sq];
    for (int i = Compass::get_dir_start_index(ch_cst::ROOK); i < Compass::get_dir_end_index(ch_cst::ROOK); i++)
    {
        for (int step = 1; step <= Compass::edge_distance64x8[sq][i]; step++)
        {
            if (Bitboard::contains_square(*chess.bb_by_color[chess.aci], sq+step*DIRS[i]))
                break;
            if (Bitboard::contains_square(pinned_pieces, sq)
            && !Bitboard::contains_square(pin_ray_moves, sq+step*DIRS[i]))
                continue;
            rook_moves.push_back(Move(sq, sq+step*DIRS[i]));
            if (Bitboard::contains_square(*chess.bb_by_color[1-chess.aci], sq+step*DIRS[i]))
                break;
        }
    }
    if (!in_check)
        return rook_moves;
    std::vector<Move> legal_moves;
    for (Move m : rook_moves)
    {
        if (m.end() == check_square || Bitboard::contains_square(check_ray, m.end()))
            legal_moves.push_back(m);
    }
    return legal_moves;
}

std::vector<Move> MoveGenerator::gen_king_piece_moves(int sq)
{
    std::vector<Move> king_moves;
    U64 moves = Compass::king_attacks[sq] & ~*chess.bb_by_color[chess.aci] & ~gen_op_attack_mask();
    // normal moves
    for (int endsq = std::max(0, sq-9); endsq < std::min(64, sq+9); endsq++)
    {
        if (!Bitboard::contains_square(moves, endsq))
            continue;
        king_moves.push_back(Move(sq, endsq));
    }
    // castle moves
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

/*
 * Performance test method
 * @param depth number of ply to test
 * @param log true if we should print nodes per move in root node
 * @param initial_pos true if we are testing the initial position.
 *        if (initial_pos && depth < 16) we can print known target values.
 */
U64 MoveGenerator::perft_root(int depth, bool log, bool initial_pos) {
    if (depth == 0)
        return 1;
    std::string perft_results[16] = {
        "1",
        "20",
        "400",
        "8902",
        "197281",
        "4865609",
        "119060324",
        "3195901860",
        "84998978956",
        "2439530234167",
        "69352859712417",
        "2097651003696806",
        "62854969236701747",
        "1981066775000396239",
        "61885021521585529237",
        "2015099950053364471960"
    };
    Timer t;
    U64 nodes = 0;
    std::cout << "Starting perft(" << depth << "):" << std::endl;
    std::vector<Move> moves = gen_moves();
    for (int mvidx = 0; mvidx < moves.size(); mvidx++)
    {
        Move mv = moves.at(mvidx);
        if (log)
            std::cout << mvidx + 1 << "/" << moves.size() << ": (" << ch_cst::square_to_string[mv.start()] << ", " << ch_cst::square_to_string[mv.end()] << ") ";
        chess.make_move(mv);
        int i = perft(depth - 1);
        if (log)
            std::cout << i << std::endl;
        nodes += i;
        chess.unmake_move(1);
    }
    std::cout << nodes << " moves found in " << t.elapsed() << " seconds." << std::endl;
    if (initial_pos && depth < 16)
        std::cout << perft_results[depth] << " target value." << std::endl;
    return nodes;
}

U64 MoveGenerator::perft(int depth) {
    if (depth == 0)
        return 1;
    U64 nodes = 0;
    for (Move mv : gen_moves()) {
        chess.make_move(mv);
        nodes += perft(depth - 1);
        chess.unmake_move(1);
    }
    return nodes;
}
