#include "Player.h"

Player::Player(float mob_percent) : search_log("iter_search_log", 0) {
    var_mobility_weight = MOB_CONST * mob_percent;
}

/*
 * Method to find a move using an iterative search
 * @param depth target depth to search
 * @param nodes U64& to count the number of positions searched
 * @param test true if special debug information should be printed
 * @return the best move found in the search
 */
move Player::iterative_search(int depth, U64& nodes, bool test) {
    move moves[MAXMOVES] = {};
    MoveGenerator mgen(Chess::state());
    mgen.gen_moves(moves);
    bool extended = false;
    // depth += BB::count_bits(Chess::state()->bb_occ) < 6;
    if (TTable::fill_ratio() > 0.7) TTable::clear();

    fmt::print("Beginning search at depth ");
    for (int iter = 1; iter <= depth; iter++) {
        fmt::print("{} . . . ", iter);
        float high_score = -99.99f;
        for (int mvidx = 0; mvidx < moves[MAXMOVES - 1]; mvidx++) {
            Chess::push_move(moves[mvidx]);
            float score = -nega_max(iter - 1, nodes, -99.99f, -high_score, test);
            Chess::unmake_move(1);
            // print output of search
            if (test && iter == depth)
                fmt::print("\n{:>2d}/{}: {:<6} {:0.2f}",
                    mvidx + 1, moves[MAXMOVES - 1], MoveGenerator::move_san(moves[mvidx]), score);
            Move::arr_shift_right(moves, score > high_score ? mvidx : 0);
            high_score = score > high_score ? score : high_score;
        }
        if (!extended && iter == depth && high_score > var_piece_value[ch_cst::QUEEN] / 100) {
            // if we are winning by more than a queen, search a little more.
            depth += 3;
            extended = true;
        }
    }
    fmt::print(" \n");
    return moves[0];
}

/*
 * @param ch the position to be searched
 * @param depth the number of ply to search
 * @param nodes the number of positions seen so far
 * @param a the score-to-beat for future evaluations
 * @param b the minimum eval allowed by the opponent
 * @return the eval of the most favorable end node
 */
float Player::nega_max(int depth, U64& nodes, float alpha, float beta, bool test) {
    Chess& ch = *Chess::state();
    MoveGenerator mgen(ch);
    move moves[MAXMOVES] = {};
    mgen.gen_moves(moves);
    nodes++;

    // check for mate
    if (!moves[MAXMOVES - 1])
        return eval(depth);

    // check the t-table for a best move
    if (ch.repetitions() > 2)
        return 0.0f;

    Entry prev = TTable::probe(ch.zhash);
    // if the stored depth was >= remaining search depth, use that result
    if (prev.depth >= depth) {
        TTable::hits++;
        if (prev.flag == Entry::FLAG_EXACT)
            return prev.score;
        else if (prev.flag == Entry::FLAG_ALPHA && prev.score <= alpha)
            return alpha;
        else if (prev.flag == Entry::FLAG_BETA && prev.score >= beta)
            return beta;
        TTable::hits--;
    }

    if (!depth) {
        nodes--;
        float score = quiescence_search(depth, nodes, alpha, beta, test);
        TTable::add_item(ch.zhash, depth, Entry::FLAG_EXACT, alpha);
        return score;
    }

    // prioritize searching previous best moves
    move best = 0;
    if (prev.best) {
        TTable::hits++;
        int best_pos = 0;
        while (moves[best_pos] != prev.best)
            best_pos++;
        Move::arr_shift_right(moves, best_pos);
    }

    for (int mvidx = 0; mvidx < moves[MAXMOVES - 1]; mvidx++) {
        Chess::push_move(moves[mvidx]);
        float score = -nega_max(depth - 1, nodes, -beta, -alpha, test);
        Chess::unmake_move(1);
        if (score >= beta) {
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
float Player::quiescence_search(int depth, U64& nodes, float alpha, float beta, bool test) {
    Chess& ch = *Chess::state();
    MoveGenerator mgen(ch);
    move moves[MAXMOVES] {};
    mgen.gen_moves(moves);
    float stand_pat = eval(depth, test);
    nodes++;
    if (!moves[MAXMOVES - 1] || stand_pat >= beta)
        return stand_pat;

    // Delta pruning: if a huge swing (> 1 queen)
    // is not enough to improve the position, give up
    const float DELTA = 15;
    if (stand_pat < alpha - DELTA)
        return alpha;

    Entry prev = TTable::probe(ch.zhash);
    // if the stored depth was >= remaining search depth, use that result
    if (prev.depth >= depth) {
        TTable::hits++;
        if (prev.flag == Entry::FLAG_EXACT)
            return prev.score;
        else if (prev.flag == Entry::FLAG_ALPHA && prev.score <= alpha)
            return alpha; // max score of stored move is worse than alpha
        else if (prev.flag == Entry::FLAG_BETA && prev.score >= beta)
            return beta; // min score of stored move is better than beta
        TTable::hits--;
    }

    // prioritize searching previous best moves
    if (prev.best) {
        TTable::hits++;
        int best_pos = 0;
        while (moves[best_pos] != prev.best) best_pos++;
        Move::arr_shift_right(moves, best_pos);
    }

    // make captures until no captures remain, then eval
    alpha = stand_pat > alpha ? stand_pat : alpha;
    move best = 0;
    for (int mvidx = 0; mvidx < moves[MAXMOVES - 1]; mvidx++) {
        if (!BB::contains_square(ch.bb_occ, Move::end(moves[mvidx])) && Move::end(moves[mvidx]) != ch.ep_square)
            continue;
        nodes++;
        Chess::push_move(moves[mvidx]);
        float score = -quiescence_search(depth - 1, nodes, -beta, -alpha, test);
        Chess::unmake_move(1);

        // move scored >= beta (fail-high)
        // failing high means there is a "best" move, even though we can't play it
        // really the move is just "good enough", since there could be a better move
        if (score >= beta) {
            TTable::add_item(ch.zhash, depth, Entry::FLAG_BETA, beta, moves[mvidx]);
            return beta;
        }
        best = score > alpha ? moves[mvidx] : best;
        alpha = score > alpha ? score : alpha;
    }
    TTable::add_item(ch.zhash, depth, best ? Entry::FLAG_EXACT : Entry::FLAG_ALPHA, alpha, best);
    return alpha;
}

void Player::order_moves_by_piece(const move moves[MAXMOVES], move* ordered) const {
    Chess& ch = *Chess::state();
    ordered[MAXMOVES] = {};
    move hash_move = TTable::read(ch.zhash).best;
    if (hash_move) {
        ordered[ordered[MAXMOVES - 1]] = hash_move;
        ordered[MAXMOVES - 1]++;
    }
    for (int piece = ch_cst::KING; piece >= ch_cst::PAWN; piece--)
        for (int i = 0; i < moves[MAXMOVES - 1]; i++) {
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
float Player::eval(int mate_offset, bool test) const {
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
    float middlegame_weight = BB::count_bits(ch.bb_occ) / var_endgame_weight;
    float score = eval_position(middlegame_weight);

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
    if (test) fmt::print("net moves: {:<5} | mobility: {:<4.2f} | score: {:<4.2f}\n",
        net_mobility, mobility_score/100, score);
    return score;
}

float Player::eval_position(float middlegame_weight) const {
    // white pieces & position
    float score  = eval_piece(middlegame_weight, ch_cst::PAWN, false);
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

/*
 * @returns sum(the piece's value + the piece's positional value) for a given color
 */
float Player::eval_piece(float middlegame_weight, int piece, bool is_black) const {
    float score = 0;
    U64 pieces = *Chess::state()->bb_color[is_black] & *Chess::state()->bb_piece[piece];
    int piece_value = var_piece_value[piece];
    while (pieces) {
        // x & -x masks the LS1B
        score += piece_value + PieceLocationTables::complex_read(piece, 63 - BB::lz_count(pieces & 0-pieces), middlegame_weight, is_black);
        // now clear that LS1B
        pieces &= pieces - 1;
    }
    return score;
}

float Player::king_safety(bool is_black) const {
    Chess ch = *Chess::state();
    U64 kattacks = Compass::king_attacks[ch.find_king(is_black)];

    return 0.25 * BB::count_bits(kattacks);
}
