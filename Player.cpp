#include "Player.h"

Player::Player(float mob_percent)
{
    var_mobility_weight = 0.05f * mob_percent;
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
move Player::iterative_search(Chess ch, int depth, U64& nodes, bool test)
{
    // if (BB::num_bits_flipped(ch.bb_occ) <= 6)
    //     depth = depth + 2;
    // if (BB::num_bits_flipped(ch.bb_occ) == 3)
    //     depth = depth + 3;
    MoveGenerator mgen(ch);
    float high_score = -99.99f;
    move moves[120] = {};
    mgen.gen_moves(moves);
    if (moves[119] == 1)
        return moves[0];
    order_moves_by_piece(ch, moves, moves);
    move best_move = moves[0];
    nodes = 0;

    // Iterative search loop
    for (int iter = 1; iter <= depth; iter++)
    {
        for (uint8_t mvidx = 0; mvidx < moves[119]; mvidx++)
        {
            move mv = moves[mvidx];
            ch.make_move(mv);
            float score = -nega_max(ch, iter - 1, nodes, -99.99f, -high_score, test);
            best_move  = score > high_score ? mv    : best_move;
            high_score = score > high_score ? score : high_score;
            ch.unmake_move(1);

            // print output of search
            if (test && iter == depth)
                fmt::print("{:>2d}/{}: {:<6} {:0.2f}\n",
                    mvidx + 1, moves[119], MoveGenerator::move_san(ch, mv), score);
        }
        TTable::add_item(ch.zhash, depth, Entry::FLAG_ALPHA, high_score, best_move);
    }
    return best_move;
}

/*
 * @param ch the position to be evaluated
 * @param depth the number of ply to search
 * @param nodes the number of positions seen so far
 * @param a the score-to-beat for future evaluations
 * @param b the minimum eval allowed by the opponent
 * @return the eval of the most favorable end node
 */
float Player::nega_max(Chess& ch, int depth, U64& nodes, float alpha, float beta, bool test)
{
    MoveGenerator mgen(ch);
    move moves[120] = {};
    mgen.gen_moves(moves);
    // check for mate
    if (!moves[119])
        return eval(ch);
    // check the t-table for a best move
    if (ch.repetitions() > 2)
        return 0.0f;
    // if the stored depth was >= remaining search depth, use that result
    Entry prev = TTable::probe(ch.zhash);
    if (!depth || (prev.flag && prev.depth >= depth))
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
            // float score = eval(ch, depth, test);
            float score = quiescence_search(ch, depth, nodes, alpha, beta, test);
            TTable::add_item(ch.zhash, depth, Entry::FLAG_EXACT, score);
            return score;
        }
    }

    nodes++;
    order_moves_by_piece(ch, moves, moves);
    move best_move = 0;
    for (uint8_t mvidx = 0; mvidx < moves[119]; mvidx++)
    {
        ch.make_move(moves[mvidx]);
        float score = -nega_max(ch, depth - 1, nodes, -beta, -alpha, test);
        ch.unmake_move(1);
        if (score >= beta)
        {
            TTable::add_item(ch.zhash, depth, Entry::FLAG_BETA, beta);
            return beta;
        }
        alpha = score > alpha ? score : alpha;
    }
    TTable::add_item(ch.zhash, depth, Entry::FLAG_ALPHA, alpha);
    return alpha;
}

/*
 *
 */
float Player::quiescence_search(Chess& ch, int depth, U64& nodes, float alpha, float beta, bool test)
{
    MoveGenerator mgen(ch);
    move moves[120] {};
    mgen.gen_moves(moves);
    float stand_pat = eval(ch, depth, test);
    if (!moves[119] || stand_pat > beta)
        return stand_pat;

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

    // Delta pruning: if a huge swing (> 1 queen)
    // is not enough to improve the position, give up
    // const float DELTA = 15;
    // if (stand_pat < alpha - DELTA)
    // {
    //     std::cout << "DELTA PRUNE" << std::endl;
    //     return alpha;
    // }

    nodes++;
    alpha = stand_pat > alpha ? stand_pat : alpha;
    order_moves_by_piece(ch, moves, moves);
    // make captures until no captures remain, then eval
    for (uint8_t mvidx = 0; mvidx < moves[119]; mvidx++)
    {
        if (!BB::contains_square(ch.bb_occ, Move::end(moves[mvidx])) && Move::end(moves[mvidx]) != ch.ep_square)
            continue;
        nodes++;
        ch.make_move(moves[mvidx]);
        float score = -quiescence_search(ch, depth - 1, nodes, -beta, -alpha, test);
        ch.unmake_move(1);
        if (score >= beta)
        {
            TTable::add_item(ch.zhash, depth, Entry::FLAG_BETA, beta);
            return beta;
        }
        alpha = score > alpha ? score : alpha;
    }
    return alpha;
}

bool Player::move_is_check(Chess& ch, move mv)
{
    ch.make_move(mv);
    MoveGenerator check_gen(ch);
    ch.unmake_move(1);
    return MoveGenerator(ch).in_check;
}

void Player::order_moves_by_piece(Chess& ch, const move* moves, move* out) const
{
    move ordered[120] = {};
    uint8_t idx = 0;
    move hash_move = TTable::read(ch.zhash).best;
    if (hash_move)
    {
        ordered[idx] = hash_move;
        idx++;
    }
    for (uint8_t piece = ch_cst::KING; piece >= ch_cst::PAWN; piece--)
    {
        for (uint8_t i = 0; i < 120; i++)
        {
            move mv = moves[i];
            if (!BB::contains_square(*ch.bb_piece[piece], Move::start(mv)) || mv == hash_move)
                continue;
            ordered[idx] = mv;
            idx++;
        }
    }
    out = ordered;
}

float Player::eval(Chess& ch, int mate_offset, bool test)
{
    float material_score = 0, positional_score = 0;
    MoveGenerator eval_gen(ch);
    move moves[120] = {};
    eval_gen.gen_moves(moves);

    // is the game over?
    if (!moves[119])
        // if it is a stalemate, return 0
        return eval_gen.in_check ? ((-99.99f - mate_offset) * (ch.black_to_move ? -1 : 1)) : 0.0f;
    // detect threefold repetition
    if (ch.repetitions() >= 3) return 0;

    // game isn't over, eval the position
    // endgame interpolation
    float middlegame_weight = BB::num_bits_flipped(ch.bb_occ) / var_endgame_weight;

    // white pieces & position
    for (uint8_t p = ch_cst::PAWN; p <= ch_cst::KING; p++)
    {
        U64 pieces = ch.bb_white & *ch.bb_piece[p];
        while (pieces)
        {
            int sq = 63 - BB::lead_0s(pieces & 0-pieces);
            material_score += var_piece_value[p];
            positional_score += PieceLocationTables::complex_read(p, sq, middlegame_weight, false);
            pieces &= pieces - 1;
        }
    }
    // black pieces and position
    for (uint8_t p = ch_cst::PAWN; p <= ch_cst::KING; p++)
    {
        U64 pieces = ch.bb_black & *ch.bb_piece[p];
        while (pieces)
        {
            int sq = 63 - BB::lead_0s(pieces & 0-pieces);
            material_score -= var_piece_value[p];
            positional_score -= PieceLocationTables::complex_read(p, sq, middlegame_weight, true);
            pieces &= pieces - 1;
        }
    }

    float score = material_score + (positional_score / 100.0f);
    // adjust the eval so the player to move is positive
    score = ch.black_to_move ? -score : score;

    // mobility score:
    ch.black_to_move = !ch.black_to_move;
    int net_mobility = moves[119];
    eval_gen.gen_moves(moves);
    net_mobility -= moves[119];
    ch.black_to_move = !ch.black_to_move;
    float mobility_score = net_mobility * var_mobility_weight * middlegame_weight;
    score += mobility_score;

    // round to the nearest hundreth
    score = std::round(score * 100.0f) / 100.0f;
    // if (test) fmt::print("net moves: {:<3} | mobility: {:<4.2f} | score: {:<4.2f} | ratio: {:<4.2f}\n",
    //     net_mobility, mobility_score, score, mobility_score / (score - mobility_score));
    return score;
}
