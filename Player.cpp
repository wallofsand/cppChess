#include "Player.h"

/*
 * Method to find a move using an iterative search
 * @param ch the position to search
 * @param search_log class describing how to log the search
 * @param depth goal depth to search
 * @param nodes U64& variable used to count the number of positions searched
 * @param test true if special debug information should be printed
 * @return the best move found in the search
 */
move Player::iterative_search(Chess::State& ch, int8_t depth, U64& nodes, bool test)
{
    SearchLogger search_log("iter_search_log", 1);
    depth = std::max((int) depth, 1);
    MoveGenerator mgen(ch);
    float high_score = -99.99f;
    std::vector<move> moves = mgen.gen_moves();
    if (moves.size() == 1)
        return moves.at(0);
    moves = order_moves_by_piece(ch, moves);
    move best_move = 0;
    nodes = 0;

    // Iterative search loop
    for (int8_t iter = 1; iter <= depth; iter++)
    {
        int best_idx = 0;
        std::vector<move> temp;
        for (int idx = 0; idx < (int) moves.size(); idx++)
        {
            move m = moves.at(idx);
            Chess::make_move(ch, m);
            float score = -nega_max(ch, search_log, iter - 1, nodes, -99.99f, -high_score, test);
            temp.insert((score > high_score ? temp.begin() : temp.end()), m);
            best_idx = score > high_score ? idx : best_idx;
            best_move = score > high_score ? m : best_move;
            high_score = std::max(score, high_score);
            Chess::unmake_move(ch, 1);

            // print output of search
            if (test && iter == depth)
                fmt::print("{:>2d}/{}: {:<6} {:0.2f}\n", idx+1, moves.size(), mgen.move_san(m), score);
        }
        TTable::add_item(ch.zhash, depth, Entry::FLAG_ALPHA, high_score, best_move);
        temp.push_back(best_move);
        for (int idx = 0; idx < (int) moves.size(); idx++)
        {
            if (idx == best_idx)
                continue;
            temp.push_back(moves.at(idx));
        }
        moves.clear();
        for (move m : temp)
            moves.push_back(m);
    }
    return best_move;
}

move Player::get_move(Chess::State& ch, SearchLogger& search_log, int8_t depth, U64& nodes, bool test)
{
    depth = std::max((int) depth, 1);
    MoveGenerator mgen(ch);
    float high_score = -99.99f;
    std::vector<move> moves = mgen.gen_moves();
    if (moves.size() == 1)
        return moves.at(0);
    moves = order_moves_by_piece(ch, moves);
    move best_move = 0;
    nodes = 0;

    // hash the position and check the t-table for a best move
    // if the stored depth was >= remaining search depth, use that result
    Entry prev = TTable::probe(ch.zhash);
    if (prev.depth >= depth)
    {
        TTable::hits++;
        return prev.best;
    }

    for (move m : moves)
    {
        Chess::make_move(ch, m);
        float score = -nega_max(ch, search_log, depth - 1, nodes, -99.99f, -high_score, test);
        best_move = score > high_score ? m : best_move;
        high_score = std::max(score, high_score);
        Chess::unmake_move(ch, 1);
    }
    TTable::add_item(ch.zhash, depth, Entry::FLAG_ALPHA, high_score, best_move);
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
float Player::nega_max(Chess::State& ch, SearchLogger& search_log, int8_t depth, U64& nodes, float alpha, float beta, bool test)
{
    MoveGenerator mgen(ch);
    std::vector<move> moves = mgen.gen_moves();
    // check the t-table for a best move
    if (Chess::repetitions(ch) > 2)
        return 0.0f;
    // if the stored depth was >= remaining search depth, use that result
    Entry prev = TTable::probe(ch.zhash);
    if (!depth || prev.depth >= depth)
    {
        TTable::hits++;
        if (prev.flag == Entry::FLAG_EXACT)
            return prev.score;
        else if (prev.flag == Entry::FLAG_ALPHA && prev.score <= alpha)
            return alpha;
        else if (prev.flag == Entry::FLAG_BETA && prev.score >= beta)
            return beta;
        TTable::hits--;
        if (!depth)
        {
            // float score = eval(ch, depth, test);
            float score = quiescence_search(ch, search_log, depth, nodes, alpha, beta, test);
            TTable::add_item(ch.zhash, depth, Entry::FLAG_EXACT, score);
            return score;
        }
    }

    if (!moves.size())
        return eval(ch);

    nodes++;
    moves = order_moves_by_piece(ch, moves);
    move best_move = 0;
    for (move mv : moves)
    {
        Chess::make_move(ch, mv);
        float score = -nega_max(ch, search_log, depth - 1, nodes, -beta, -alpha, test);
        Chess::unmake_move(ch, 1);
        if (score >= beta)
        {
            TTable::add_item(ch.zhash, depth, Entry::FLAG_BETA, beta);
            return beta;
        }
        alpha = std::max(alpha, score);
    }
    TTable::add_item(ch.zhash, depth, Entry::FLAG_ALPHA, alpha);
    return alpha;
}

/*
 *
 */
float Player::quiescence_search(Chess::State& ch, SearchLogger& search_log, int8_t depth, U64& nodes, float alpha, float beta, bool test)
{
    MoveGenerator mgen(ch);
    std::vector<move> moves = mgen.gen_moves();
    float stand_pat = eval(ch, depth, test);
    if (!moves.size() || stand_pat > beta)
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
    const float DELTA = var_piece_value[ch_cst::QUEEN];
    if (stand_pat < alpha - DELTA)
        return alpha;

    nodes++;
    alpha = std::max(alpha, stand_pat);
    moves = order_moves_by_piece(ch, moves);
    // make captures until no captures remain, then eval
    for (move mv : moves)
    {
        if (!BB::contains_square(ch.bb_occ, Move::end(mv)) && Move::end(mv) != ch.ep_square)
            continue;
        nodes++;
        Chess::make_move(ch, mv);
        float score = -quiescence_search(ch, search_log, depth - 1, nodes, -beta, -alpha, test);
        Chess::unmake_move(ch, 1);
        if (score >= beta)
        {
            TTable::add_item(ch.zhash, depth, Entry::FLAG_BETA, beta);
            return beta;
        }
        alpha = std::max(alpha, score);
    }
    return alpha;
}

std::vector<move> Player::order_moves_by_piece(Chess::State& ch, std::vector<move> moves)
{
    std::vector<move> ordered;
    for (int piece = ch_cst::KING; piece >= ch_cst::PAWN; piece--)
    {
        for (move mv: moves)
        {
            if (!BB::contains_square(*ch.bb_piece[piece], Move::start(mv)))
                continue;
            ordered.push_back(mv);
        }
    }
    return ordered;
}

float Player::eval(Chess::State& ch, int8_t mate_offset, bool test)
{
    float material_score = 0;
    float positional_score = 0;
    const float CHECKMATED = 99.99f;
    MoveGenerator eval_gen(ch);
    std::vector<move> move_list = eval_gen.gen_moves(false);

    // is the game over?
    if (!move_list.size())
    {
        if(eval_gen.in_check)
            return ch.black_to_move ? CHECKMATED + mate_offset : -(CHECKMATED + mate_offset);
        // If it is a stalemate return 0
        return 0.0f;
    } else {
        // game isn't over, eval the position
        // detect threefold repetition
        if (Chess::repetitions(ch) >= 3) return 0;

        // endgame interpolation
        float middlegame_weight = BB::num_bits_flipped(ch.bb_occ) / var_endgame_weight;
        U64 pieces = ch.bb_white;
        while (pieces)
        {
            // x & -x masks the LS1B
            int sq = 63 - (BB::lead_0s(pieces & 0-pieces));
            material_score += var_piece_value[Chess::piece_at(ch, sq)];
            positional_score += PieceLocationTables::complex_read(Chess::piece_at(ch, sq), sq, middlegame_weight, false);
            // now clear that LS1B
            pieces &= pieces - 1;
        }
        pieces = ch.bb_black;
        while (pieces)
        {
            // x & -x masks the LS1B
            int sq = 63 - (BB::lead_0s(pieces & 0-pieces));
            material_score -= var_piece_value[Chess::piece_at(ch, sq)];
            positional_score -= PieceLocationTables::complex_read(Chess::piece_at(ch, sq), sq, middlegame_weight, true) ;
            // now clear that LS1B
            pieces &= pieces - 1;
        }
    }
    float score = material_score + (positional_score / 100.0f);
    // adjust the eval so the player to move is positive
    score = ch.black_to_move ? -score : score;

    // mobility score:
    ch.black_to_move = 1 - ch.black_to_move;
    int net_mobility = (int) move_list.size() - (int) eval_gen.gen_moves().size();
    ch.black_to_move = 1 - ch.black_to_move;
    float mobility_score = var_mobility_weight;
    score += mobility_score;

    // round to the nearest hundreth
    score = std::round(score * 100.0f) / 100.0f;
    if (test) fmt::print("net moves: {:<3} | mobility: {:<4.2f} | score: {:<4.2f} | ratio: {:<4.2f}\n", net_mobility, mobility_score, score, mobility_score/score);
    return score;
}
