#include "Player.h"

Player::Player(float mob_percent)
{
    var_mobility_weight = MOB_CONST * mob_percent;
    SearchLogger search_log("iter_search_log", 0);
}

/*
 * Method to find a move using an iterative search
 * @param ch the position to search
 * @param depth target depth to search
 * @param nodes U64& to count the number of positions searched
 * @param test true if special debug information should be printed
 * @return the best move found in the search
 */
move Player::iterative_search(int depth, U64& nodes, bool test)
{
    // get the current game state
    Chess& ch = *Chess::state();

    MoveGenerator mgen(ch);
    float high_score = -99.99f;
    move moves[MAXMOVES] = {};
    mgen.gen_moves(moves);
    if (moves[MAXMOVES - 1] == 1)
        return moves[0];
    move best = moves[0];
    nodes = 0;

    fmt::print("Beginning search at depth ");
    // Iterative search loop
    for (int iter = 1; iter <= depth; iter++)
    {
        fmt::print("{} . . . ", iter);
        for (int mvidx = 0; mvidx < moves[MAXMOVES - 1]; mvidx++)
        {
            Chess::push_move(moves[mvidx]);
            float score = -nega_max(iter - 1, nodes, -99.99f, -high_score, test);
            best        = score > high_score ? moves[mvidx] : best;
            high_score  = score > high_score ? score        : high_score;
            ch.unmake_move(1);
            // print output of search
            if (test && iter == depth)
                fmt::print("{:>2d}/{}: {:<6} {:0.2f}\n",
                    mvidx + 1, moves[MAXMOVES - 1], MoveGenerator::move_san(moves[mvidx]), score);
        }
        TTable::add_item(ch.zhash, iter, Entry::FLAG_EXACT, high_score, best);
    }
    fmt::print(" \n");
    return best;
}

/*
 * @param ch the position to be searched
 * @param depth the number of ply to search
 * @param nodes the number of positions seen so far
 * @param a the score-to-beat for future evaluations
 * @param b the minimum eval allowed by the opponent
 * @return the eval of the most favorable end node
 */
float Player::nega_max(int depth, U64& nodes, float alpha, float beta, bool test)
{
    Chess& ch = *Chess::state();
    MoveGenerator mgen(ch);
    move moves[MAXMOVES] = {};
    mgen.gen_moves(moves);
    // check for mate
    if (!moves[MAXMOVES - 1])
        return eval(depth);
    // check the t-table for a best move
    if (ch.repetitions() > 2)
        return 0.0f;
    // if the stored depth was >= remaining search depth, use that result
    Entry prev = TTable::probe(ch.zhash);
    if (!depth || prev.depth > depth)
    {
        if (prev.flag && prev.depth >= depth)
        {
            TTable::hits++;
            if (prev.flag == Entry::FLAG_EXACT)
                return prev.score;
            else if (prev.flag == Entry::FLAG_ALPHA && prev.score <= alpha)
                return alpha;
            else if (prev.flag == Entry::FLAG_BETA && prev.score >= beta)
                return beta;
            TTable::hits--;
        }
        if (!depth)
        {
            float score = quiescence_search(depth, nodes, alpha, beta, test);
            return score;
        }
    }

    nodes++;
    move best = 0;
    for (int mvidx = 0; mvidx < moves[MAXMOVES - 1]; mvidx++)
    {
        Chess::push_move(moves[mvidx]);
        float score = -nega_max(depth - 1, nodes, -beta, -alpha, test);
        ch.unmake_move(1);
        if (score >= beta)
        {
            TTable::add_item(ch.zhash, depth, Entry::FLAG_BETA, beta);
            return beta;
        }
        alpha = score > alpha ? score : alpha;
    }
    TTable::add_item(ch.zhash, depth, best ? Entry::FLAG_EXACT : Entry::FLAG_ALPHA, alpha, best);
    return alpha;
}

/*
 * Method to search only capturing moves
 * @return the eval() result of the highest scoring node
 */
float Player::quiescence_search(int depth, U64& nodes, float alpha, float beta, bool test)
{
    Chess& ch = *Chess::state();
    MoveGenerator mgen(ch);
    move moves[MAXMOVES] {};
    mgen.gen_moves(moves);
    float stand_pat = eval(depth, test);
    if (!moves[MAXMOVES - 1] || stand_pat >= beta)
        return stand_pat;

    // Delta pruning: if a huge swing (> 1 queen)
    // is not enough to improve the position, give up
    const float DELTA = 15;
    if (stand_pat < alpha - DELTA)
        return alpha;

    // if the stored depth was >= remaining search depth, use that result
    Entry prev = TTable::probe(ch.zhash);
    if (prev.depth >= depth)
    {
        TTable::hits++;
        if (prev.flag == Entry::FLAG_EXACT)
            return prev.score;
        else if (prev.flag == Entry::FLAG_ALPHA && prev.score <= alpha)
            return alpha;
        else if (prev.flag == Entry::FLAG_BETA && prev.score >= beta)
            return beta;
        TTable::hits--;
    }

    // make captures until no captures remain, then eval
    nodes++;
    alpha = stand_pat > alpha ? stand_pat : alpha;
    move best = 0;
    for (int mvidx = 0; mvidx < moves[MAXMOVES - 1]; mvidx++)
    {
        if (!BB::contains_square(ch.bb_occ, Move::end(moves[mvidx])) && Move::end(moves[mvidx]) != ch.ep_square)
            continue;
        nodes++;
        Chess::push_move(moves[mvidx]);
        float score = -quiescence_search(depth - 1, nodes, -beta, -alpha, test);
        ch.unmake_move(1);
        if (score >= beta)
        {
            TTable::add_item(ch.zhash, depth, Entry::FLAG_BETA, beta);
            return beta;
        }
        best = score > alpha ? moves[mvidx] : best;
        alpha = score > alpha ? score : alpha;
    }
    TTable::add_item(ch.zhash, depth, best ? Entry::FLAG_EXACT : Entry::FLAG_ALPHA, alpha, best);
    return alpha;
}

void Player::order_moves_by_piece(const move* moves, move* ordered) const
{
    Chess& ch = *Chess::state();
    ordered[MAXMOVES] = {};
    move hash_move = TTable::read(ch.zhash).best;
    if (hash_move)
    {
        ordered[ordered[MAXMOVES - 1]] = hash_move;
        ordered[MAXMOVES - 1]++;
    }
    for (int piece = ch_cst::KING; piece >= ch_cst::PAWN; piece--)
        for (int i = 0; i < moves[MAXMOVES - 1]; i++)
        {
            if (!BB::contains_square(*ch.bb_piece[piece], Move::start(moves[i])) || moves[i] == hash_move)
                continue;
            ordered[ordered[MAXMOVES - 1]] = moves[i];
            ordered[MAXMOVES - 1]++;
        }
}

/*
 * Method to evaluate a given position
 * @param ch the position to evaluate
 * @param mate_offset the remaining depth of search
 *        mating player will maximize this value to find early mates
 * @param test print some debug/logging info
 */
float Player::eval(int mate_offset, bool test)
{
    Chess& ch = *Chess::state();
    MoveGenerator eval_gen(ch);
    move moves[MAXMOVES] = {};
    eval_gen.gen_moves(moves);

    // is the game over/detect threefold repetition
    if (!moves[MAXMOVES - 1] || ch.repetitions() >= 3)
        // if it is a stalemate, return 0
        return eval_gen.in_check ? (-99.99f - mate_offset) : 0;

    // game isn't over, eval the position:
    // endgame interpolation
    float middlegame_weight = BB::num_bits_flipped(ch.bb_occ) / var_endgame_weight;
    float score = Player::eval_position(middlegame_weight);

    // adjust the eval so the player to move is positive
    score = ch.black_to_move ? -score : score;

    // mobility score:
    ch.black_to_move = !ch.black_to_move;
    int net_mobility = moves[MAXMOVES - 1];
    MoveGenerator op_gen(Chess::state());
    op_gen.gen_moves(moves);
    net_mobility -= moves[MAXMOVES - 1];
    ch.black_to_move = !ch.black_to_move;

    // mobility is worth less in the endgame
    float mobility_score = net_mobility * var_mobility_weight * middlegame_weight;
    score += mobility_score;

    // round to the nearest hundreth
    score = std::round(score) / 100;
    if (test) fmt::print("net moves: {:<3} | mobility: {:<4.2f} | score: {:<4.2f}\n",
        net_mobility, mobility_score/100, score);
    return score;
}

float Player::eval_position(float middlegame_weight)
{
    // white pieces & position
    float score = eval_piece(middlegame_weight, ch_cst::PAWN, false);
    score += eval_piece(middlegame_weight, ch_cst::KNIGHT, false);
    score += eval_piece(middlegame_weight, ch_cst::BISHOP, false);
    score += eval_piece(middlegame_weight, ch_cst::ROOK, false);
    score += eval_piece(middlegame_weight, ch_cst::QUEEN, false);
    score += eval_piece(middlegame_weight, ch_cst::KING, false);

    // black pieces and position
    score -= eval_piece(middlegame_weight, ch_cst::PAWN, true);
    score -= eval_piece(middlegame_weight, ch_cst::KNIGHT, true);
    score -= eval_piece(middlegame_weight, ch_cst::BISHOP, true);
    score -= eval_piece(middlegame_weight, ch_cst::ROOK, true);
    score -= eval_piece(middlegame_weight, ch_cst::QUEEN, true);
    score -= eval_piece(middlegame_weight, ch_cst::KING, true);
    return score;
}

float Player::eval_piece(float middlegame_weight, int piece, bool is_black)
{
    Chess& ch = *Chess::state();
    float material_score = 0;
    float positional_score = 0;
    U64 pieces = *ch.bb_color[is_black] & *ch.bb_piece[piece];
    while (pieces)
    {
        int sq = 63 - BB::lz_count(pieces & 0-pieces);
        material_score += var_piece_value[piece];
        positional_score += PieceLocationTables::complex_read(piece, sq, middlegame_weight, is_black);
        pieces &= pieces - 1;
    }
    return material_score + positional_score;
}
