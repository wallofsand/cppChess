#include "Player.h"

Move Player::get_move(Chess& ch, SearchLogger& search_log, int depth, U64& nodes, bool test)
{
    Timer search_timer;
    depth = std::max(depth, 1);
    MoveGenerator mgen(ch);
    float high_score = -99.99f;
    Move best_move = Move(0, 0);
    std::vector<Move> moves = mgen.gen_moves();
    moves = order_moves_by_piece(ch, moves);
    if (moves.size() == 1)
        return moves.at(0);
    nodes = 0;

    // hash the position and check the t-table for a best move
    // long zhash = ch.hash();
    // if the stored depth was >= remaining search depth, use that result
    // if (TTable::read(zhash).depth >= depth)
    // {
    //     TTable::hits++;
    //     return TTable::read(zhash).best;
    // }

    for (Move m : moves)
    {
        ch.make_move(m);
        float score = -nega_max(ch, search_log, depth - 1, nodes, -99.99f, -high_score, test);
        if (score > high_score)
        {
            high_score = score;
            best_move = m;
       }
        ch.unmake_move(1);
    }
    // if (search_timer.elapsed() >= 1)
    //     nodes = nodes / search_timer.elapsed();
    // TTable::entry(zhash, depth, Position::FLAG_ALPHA, high_score, best_move);
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
float Player::nega_max(Chess& ch, SearchLogger& search_log, int depth, U64& nodes, float alpha, float beta, bool test)
{
    // hash the position and check the t-table for a best move
    // long zhash = ch.hash();
    // if the stored depth was >= remaining search depth, use that result
    if (!depth) // || TTable::read(zhash).depth >= depth)
    {
        // Position prev = TTable::read(zhash);
        // TTable::hits++;
        // if (prev.flag == Position::FLAG_EXACT)
        //     return TTable::read(zhash).score;
        // else if (prev.flag == Position::FLAG_ALPHA && prev.score <= alpha)
        //     return alpha;
        // else if (prev.flag == Position::FLAG_BETA && prev.score >= beta)
        //     return beta;
        // TTable::hits--;
        if (!depth)
        {
            float score = eval(ch, depth, test);
            // float score = quiescence_search(ch, search_log, depth, nodes, alpha, beta, test);
            // TTable::entry(zhash, depth, Position::FLAG_EXACT, score);
            return score;
        }
    }

    nodes++;
    MoveGenerator mgen(ch);
    std::vector<Move> moves = mgen.gen_moves();
    moves = order_moves_by_piece(ch, moves);
    if (!moves.size())
        return eval(ch);
    for (Move mv : moves)
    {
        ch.make_move(mv);
        float score = -nega_max(ch, search_log, depth - 1, nodes, -beta, -alpha, test);
        ch.unmake_move(1);
        if (score >= beta)
        {
            // TTable::entry(zhash, depth, Position::FLAG_BETA, beta);
            return beta;
        }
        alpha = std::max(alpha, score);
    }
    // TTable::entry(zhash, depth, Position::FLAG_ALPHA, alpha);
    return alpha;
}

/*
 *
 */
float Player::quiescence_search(Chess& ch, SearchLogger& search_log, int depth, U64& nodes, float alpha, float beta, bool test)
{
    nodes++;
    MoveGenerator mgen(ch);
    float stand_pat = eval(ch, depth, test);
    std::vector<Move> moves = mgen.gen_moves();
    if (!moves.size() || stand_pat > beta)
        return stand_pat;

    // hash the position and check the t-table for a best move
    // long zhash = ch.hash();
    // if the stored depth was >= remaining search depth, use that result
    // if (TTable::read(zhash).depth >= depth)
    // {
    //     Position prev = TTable::read(zhash);
    //     TTable::hits++;
    //     if (prev.flag == Position::FLAG_EXACT)
    //         return TTable::read(zhash).score;
    //     else if (prev.flag == Position::FLAG_ALPHA && prev.score <= alpha)
    //         return alpha;
    //     else if (prev.flag == Position::FLAG_BETA && prev.score >= beta)
    //         return beta;
    //     TTable::hits--;
    // }

    // Delta pruning: if a huge swing (> 1 queen)
    // is not enough to improve the position, give up
    const float DELTA = var_piece_value[ch_cst::QUEEN];
    if (stand_pat < alpha - DELTA)
        return alpha;

    alpha = std::max(alpha, stand_pat);
    moves = order_moves_by_piece(ch, moves);
    // make captures until no captures remain, then eval
    for (Move mv : moves)
    {
        if (!BB::contains_square(ch.bb_occ, mv.end()) && mv.end() != ch.ep_square)
            continue;
        nodes++;
        ch.make_move(mv);
        float score = -quiescence_search(ch, search_log, depth - 1, nodes, -beta, -alpha, test);
        ch.unmake_move(1);
        if (score >= beta)
            return beta;
        alpha = std::max(alpha, score);
    }
    return alpha;
}

std::vector<Move> Player::order_moves_by_piece(Chess& ch, std::vector<Move> moves)
{
    std::vector<Move> ordered;
    for (int piece = ch_cst::KING; piece >= ch_cst::PAWN; piece--)
    {
        for (Move mv: moves)
        {
            if (!BB::contains_square(*ch.bb_piece[piece], mv.start()))
                continue;
            ordered.push_back(mv);
        }
    }
    return ordered;
}

float Player::eval(Chess& ch, int mate_offset, bool test)
{
    float material_score = 0;
    float positional_score = 0;
    const float MATE_IN_ZERO = 99.99f;
    MoveGenerator eval_gen(ch);
    std::vector<Move> moves = eval_gen.gen_moves();

    // is the game over?
    if (!moves.size())
    {
        if(eval_gen.in_check)
        {
            // black wins, eval low
            if (ch.aci) return -(MATE_IN_ZERO + mate_offset);
            // white wins, eval high
            else return MATE_IN_ZERO + mate_offset;
        }
        // If it is a stalemate return 0
        return 0.0f;
    } else {
        // game isn't over, eval the position
        // endgame interpolation
        float middlegame_weight = BB::num_bits_flipped(ch.bb_occ) / var_middlegame_weight;
        for (int sq = 0; sq < 64; sq++) {
            if (!BB::contains_square(ch.bb_occ, sq))
                continue;
            int piece = ch.piece_at(sq);
            material_score += var_piece_value[piece] * (1 - (2 * BB::contains_square(ch.bb_black, sq)));
            positional_score += PieceLocationTables::complex_read(piece, sq, middlegame_weight, BB::contains_square(ch.bb_black, sq)) * (1 - (2 * BB::contains_square(ch.bb_black, sq)));
        }
    }
    float score = material_score + (positional_score / 100.0f);

    // mobility score:
    float mobility_score = (float) moves.size();
    ch.aci = 1 - ch.aci;
    mobility_score -= eval_gen.gen_moves().size();
    ch.aci = 1 - ch.aci;
    mobility_score *= var_mobility_weight;
    score += mobility_score;

    // round to the nearest hundreth
    score = std::round(score * 100) / 100.0f;
    // System.out.printf("mobility: %+-7.2f | score: %+-7.2f | ratio: %+-7.2f\n", mob, score, mob/score);
    // adjust the eval so the player to move is positive
    return (ch.aci) ? -score : score;
}

/*
package chess3;

import java.text.DateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.Random;

public class QuiescencePlayer extends Player {
    // separate Chess object used for thinking about moves
    private Chess test;
    Random rando = new Random();
    const int MOD = +6;
    const float MAX = 499.99f;
    const float MIN = -MAX;
    const float mateinzero = 300f;
    U64 nodes = 0;
    long startTime, time;
    float mobWeight = 0.1f;
    bool out_of_book = false;
    TranspositionTable ttable;

    Player::Player(Chess chess, TranspositionTable table) {
        ttable = table;
    }

    public void setChess(Chess chess) {
        ch = chess;
        outofbook = false;
    }

    public Move getMove(int depth) throws InterruptedException {
        if (!outofbook) {
            startTime = System.currentTimeMillis();
            System.out.println("Searching opening book . . . ");
            Move bookMove = getBookMove(Book.readABook(ch));
            if (bookMove != null) {
                System.out.println("Book move found in " + (System.currentTimeMillis() - startTime) + "ms! "
                        + ch.moveText(bookMove));
                return bookMove;
            } else {
                System.out.println("Uh oh -- I'm out of my book now . . . in " + time + "ms!");
                outofbook = true;
            }
        }
        try {
            if (depth > 0)
                return iterativeSearch(depth, MIN, MAX);
            if (countPieces(ch) < 6)
                return iterativeSearch(MOD + 2, MIN, MAX);
            return iterativeSearch(MOD, MIN, MAX);
        } catch (InterruptedException e) {
            return null;
        }
    }

    // One! Ha, ha, ha
    private int countPieces(Chess ch) {
        int count = 0;
        for (int i = 0; i < 64; i++) {
            if (!ch.isEmptySquare(i))
                count++;
        }
        return count;
    }

    // Method to convert relevant book data into a move
    // ArrayList bookData contains 3 arrays
    // 0 - ECO codes, 1 - Opening names, 2 - move strings
    private Move getBookMove(ArrayList<ArrayList<String>> bookData) {
        if (bookData == null || bookData.get(2).size() == 0)
            return null;
        ArrayList<Move> bookMoves = new ArrayList<Move>();
        for (int i = 0; i < bookData.get(2).size(); i++) {
            String nextMoveString = bookData.get(2).get(i);
            Move nextMove = null;
            int start = -1;
            int astart = -1;
            // find the square the book move points too
            int end = (new Coordinate((nextMoveString.charAt(nextMoveString.length() - 1) == '#'
                    || nextMoveString.charAt(nextMoveString.length() - 1) == '+')
                            ? nextMoveString.substring(nextMoveString.length() - 3)
                            : nextMoveString.substring(nextMoveString.length() - 2))).getSquare();
            // Find the starting square for the move
            // Look at the board and deduce which piece could make the move
            switch (nextMoveString.charAt(0)) {
            case 'N': // knight
                for (int dirIndex = Compass.getStartIndex(Chess.KNIGHT); dirIndex < Compass
                        .getEndIndex(Chess.KNIGHT); dirIndex++) {
                    if (Compass.numSquaresToEdge64x16[end][dirIndex] > 0) {
                        if (ch.board[end + Compass.directions[dirIndex]] == (Chess.KNIGHT | ch.getActiveColour())) {
                            if (start == -1)
                                start = end + Compass.directions[dirIndex];
                            else
                                astart = end + Compass.directions[dirIndex];
                            break;
                        }
                    }
                }
                // validate ambiguity
                if (astart != -1) {
                    if (Character.isDigit(nextMoveString.charAt(1))) {
                        if ((new Coordinate(
                                new String(new char[] { nextMoveString.charAt(2), nextMoveString.charAt(1) })))
                                        .getSquare() == astart) {
                            start = astart;
                        }
                    } else {
                        if (Character.isDigit(nextMoveString.charAt(2))) {
                            start = (new Coordinate(nextMoveString.substring(1, 3))).getSquare();
                        }
                    }
                }
                nextMove = new Move(start, end);
                bookMoves.add(nextMove);
                break;
            case 'B': // bishop
                for (int dirIndex = Compass.getStartIndex(Chess.BISHOP); dirIndex < Compass
                        .getEndIndex(Chess.BISHOP); dirIndex++) {
                    for (int step = 1; step <= Compass.numSquaresToEdge64x16[end][dirIndex]; step++) {
                        if (ch.board[end + step * Compass.directions[dirIndex]] == (Chess.BISHOP | ch.getActiveColour())) {
                            if (start == -1)
                                start = end + step * Compass.directions[dirIndex];
                            else
                                astart = end + step * Compass.directions[dirIndex];
                            break;
                        }
                    }
                }
                // validate ambiguity
                if (astart != -1) {
                    if (Character.isDigit(nextMoveString.charAt(1))) {
                        if ((new Coordinate(
                                new String(new char[] { nextMoveString.charAt(2), nextMoveString.charAt(1) })))
                                        .getSquare() == astart) {
                            start = astart;
                        }
                    } else {
                        if (Character.isDigit(nextMoveString.charAt(2))) {
                            start = (new Coordinate(nextMoveString.substring(1, 3))).getSquare();
                        }
                    }
                }
                nextMove = new Move(start, end);
                bookMoves.add(nextMove);
                break;
            case 'R': // rook
                for (int dirIndex = Compass.getStartIndex(Chess.ROOK); dirIndex < Compass
                        .getEndIndex(Chess.ROOK); dirIndex++) {
                    for (int step = 1; step <= Compass.numSquaresToEdge64x16[end][dirIndex]; step++) {
                        if (ch.board[end + step * Compass.directions[dirIndex]] == (Chess.ROOK | ch.getActiveColour())) {
                            if (start == -1)
                                start = end + step * Compass.directions[dirIndex];
                            else
                                astart = end + step * Compass.directions[dirIndex];
                            break;
                        }
                        else if (!ch.isEmptySquare(end + step * Compass.directions[dirIndex]))
                            break;
                    }
                }
                // validate ambiguity
                if (astart != -1) {
                    if (Character.isDigit(nextMoveString.charAt(1))) {
                        if ((new Coordinate(
                                new String(new char[] { nextMoveString.charAt(2), nextMoveString.charAt(1) })))
                                        .getSquare() == astart) {
                            start = astart;
                        }
                    } else {
                        if (Character.isDigit(nextMoveString.charAt(2))) {
                            start = (new Coordinate(nextMoveString.substring(1, 3))).getSquare();
                        }
                    }
                }
                nextMove = new Move(start, end);
                bookMoves.add(nextMove);
                break;
            case 'Q': // queen
                for (int dirIndex = Compass.getStartIndex(Chess.QUEEN); dirIndex < Compass
                        .getEndIndex(Chess.QUEEN); dirIndex++) {
                    for (int step = 1; step <= Compass.numSquaresToEdge64x16[end][dirIndex]; step++) {
                        if (ch.board[end + step * Compass.directions[dirIndex]] == (Chess.QUEEN | ch.getActiveColour())) {
                            if (start == -1)
                                start = end + step * Compass.directions[dirIndex];
                            else
                                astart = end + step * Compass.directions[dirIndex];
                            break;
                        }
                    }
                }
                // validate ambiguity
                if (astart != -1) {
                    if (Character.isDigit(nextMoveString.charAt(1))) {
                        if ((new Coordinate(
                                new String(new char[] { nextMoveString.charAt(2), nextMoveString.charAt(1) })))
                                        .getSquare() == astart) {
                            start = astart;
                        }
                    } else {
                        if (Character.isDigit(nextMoveString.charAt(2))) {
                            start = (new Coordinate(nextMoveString.substring(1, 3))).getSquare();
                        }
                    }
                }
                nextMove = new Move(start, end);
                bookMoves.add(nextMove);
                break;
            case 'K': // king
                for (int dirIndex = Compass.getStartIndex(Chess.KING); dirIndex < Compass
                        .getEndIndex(Chess.KING); dirIndex++) {
                    if (Compass.numSquaresToEdge64x16[end][dirIndex] > 0) {
                        if (ch.board[end + Compass.directions[dirIndex]] == (Chess.KING | ch.getActiveColour())) {
                            start = end + Compass.directions[dirIndex];
                            nextMove = new Move(start, end);
                            bookMoves.add(nextMove);
                        }
                    }
                }
                break;
            case 'O': // castle
                switch (nextMoveString.length()) {
                case 3: // castle kingside
                    start = 4 + (8 * 7 * ch.activeColourIndex);
                    end = start + 2;
                    nextMove = new Move(start, end, "castle");
                    bookMoves.add(nextMove);
                    break;
                case 5: // castle queenside
                    start = 4 + (8 * 7 * ch.activeColourIndex);
                    end = start - 2;
                    nextMove = new Move(start, end, "castle");
                    bookMoves.add(nextMove);
                    break;
                }
                break;
            default: // pawn
                int pawnToMove = Chess.PAWN | ch.getActiveColour();
                if (nextMoveString.charAt(1) != 'x') {
                    if (ch.board[end - Compass.pMoves[ch.activeColourIndex]] == pawnToMove) {
                        start = end - Compass.pMoves[ch.activeColourIndex];
                        nextMove = new Move(start, end);
                        bookMoves.add(nextMove);
                    } else if (ch.board[end - 2 * Compass.pMoves[ch.activeColourIndex]] == pawnToMove) {
                        start = end - 2 * Compass.pMoves[ch.activeColourIndex];
                        nextMove = new Move(start, end);
                        bookMoves.add(nextMove);
                    }
                } else {
                    start = (new Coordinate(nextMoveString.charAt(0) + Integer.toString(
                            Character.getNumericValue(nextMoveString.charAt(3)) - 1 + (2 * ch.activeColourIndex))))
                                    .getSquare();
                    nextMove = new Move(start, end);
                    bookMoves.add(nextMove);
                }
            }
        }
        // Now we have an array of moves indexed to an array of ECO codes and opening
        // names
        // Now prompt the user to select an opening from the book
//		for (int i = 0; i < bookMoves.size(); i++) {
//			System.out.printf("%-4d|", i);
//			System.out.printf("%-3s:", bookData.get(0).get(i));
//			System.out.printf("%-65s", bookData.get(1).get(i));
//			System.out.println(bookData.get(2).get(i));
//		}
        time = System.currentTimeMillis() - startTime;
//		System.out.println("Choose an opening in the console or -1 for random:");
        int randint = rando.nextInt(bookMoves.size());

        // Pick an opening by ECO code:
//		for (int i = 0; i < bookData.get(0).size(); i++) {
//			if (bookData.get(0).get(i).equalsIgnoreCase("E91"))
//				randint = i;
//		}

        Move moveSelected = bookMoves.get(randint);
        if (moveSelected.start == -1 || moveSelected.end == -1) {
            throw new IllegalArgumentException("Invalid move conversion");
            // return null;
        }

        System.out.println();
        System.out.printf("%-4d|", randint);
        System.out.printf("%-3s:", bookData.get(0).get(randint));
        System.out.printf("%-30s ", bookData.get(1).get(randint));
        System.out.println(bookData.get(2).get(randint));
        return moveSelected;
    }

    Move iterativeSearch(int depth, float alpha, float beta) throws InterruptedException {
//		Zobrist.sethits(0);
        Move lastMove = null;
        try {
            for (int i = 1; i <= depth; i++) {
                lastMove = rootNegaMax(i, alpha, beta, i == depth);
            }
            return lastMove;
        } catch (InterruptedException e) {
            return lastMove;
        }
    }

    // a stores the score-to-beat for future moves searched
    // b stores the minimum eval allowed by the opponent
    Move rootNegaMax(int depth, float alpha, float beta, boolean doLogging) throws InterruptedException {
        this.test = new Chess(ch.startpos);
        if (ch.moveHistory.size() > 0)
            for (Move m : ch.moveHistory)
                test.makeHistory(m);
        MoveGenerator mgen = new MoveGenerator(test);
        if (mgen.moves.size() == 1)
            return mgen.moves.get(0);

        // hash the position and check the t-table for a best move
        long zhash = Zobrist.hash(test);
        // if the stored depth was >= remaining search depth, use that result
        if (ttable.getDepth(zhash) >= depth) {
//			Zobrist.incrementHits();
            return ttable.getMove(zhash);
        }
        mgen.moves = orderMovesByValue(mgen.moves, test);
        long startTime = System.currentTimeMillis();
        nodes = 0;
        Move bestMove = null;
        // counter variable for printing move labels
        int moveCounter = 0;
        float score = 0;
        if (doLogging) {
            System.out.println("\nCurrent eval is " + String.format("%.2f", evalMaterial(mgen, 0)));
            System.out.println("Starting quiescence search of depth " + depth + " at " + 
                    DateFormat.getTimeInstance().format(new Date()));
        }
        // if the stored depth was >= remaining search depth, use that result
        if (ttable.getDepth(zhash) >= depth)
            return ttable.getMove(zhash);
        for (Move m : mgen.moves) {
            if (Thread.interrupted()) // We've been interrupted: no more crunching.
                throw new InterruptedException();
            test.makeHistory(m);
            moveCounter++;
            nodes++;
            long nodeStart = System.currentTimeMillis();
            long subnodes = nodes;
            if (doLogging) System.out.printf("%2d/%-2d: %-7s", moveCounter, mgen.moves.size(), ch.moveText(m));
            score = -negaMax(depth - 1, -beta, -alpha);
            if (score > alpha || bestMove == null) {
                bestMove = m;
                alpha = score;
            }
            test.unmakeMove();
            subnodes = nodes - subnodes;
            if (doLogging) System.out.printf("%+-7.2f|%8d nodes|", score, subnodes);
            long nodeTime = System.currentTimeMillis() - nodeStart;
            // Print time
            if (doLogging) {
                if (System.currentTimeMillis() - startTime > 1000) {
                    System.out.printf("%3d.%03ds |", (System.currentTimeMillis() - startTime) / 1000,
                            System.currentTimeMillis() % 1000);
                } else {
                    System.out.printf("%5dms  |", System.currentTimeMillis() - startTime);
                }
                // Print nodes/sec
                if (nodeTime > 0) {
                    double rate = subnodes / (nodeTime / 1000f);
                    System.out.printf("%10.0f nodes/sec%n", rate);
                } else
                    System.out.println();
            }
        }
        int timepassed = (int) (System.currentTimeMillis() - startTime);
        if (doLogging) {
            // Print the turn number
            System.out.print(ch.moveHistory.size() / 2 + 1);
            if (ch.getActiveColour() == Chess.WHITE)
                System.out.print(". ");
            else
                System.out.print("... ");
            // Print the chosen move
            System.out.print(ch.moveText(bestMove));
            // Print the eval of the predicted position
            System.out.printf(" \teval:\t%+.2f\t%n", alpha);
            // Print the search statistics
            System.out.print("Search of ");
            if (nodes > 999999)
                System.out.printf("%d,%03d,%03d nodes finished in ", nodes / 1000000, (nodes / 1000) % 1000, nodes % 1000);
            else if (nodes > 999)
                System.out.printf("%d,%03d nodes finished in ", (nodes / 1000) % 1000, nodes % 1000);
            else
                System.out.printf("%d nodes finished in ", nodes);
            if (timepassed < 1000) {
                System.out.printf("%dms", timepassed);
            } else
                System.out.printf("%d.%ds", timepassed / 1000, timepassed % 1000);
            float speed;
            if (timepassed > 0) {
                speed = nodes * 1000 / timepassed;
                System.out.printf(" at %.0f nodes/sec%n", speed);
            }
        }
        ttable.makePosition(zhash, depth, Position.flagALPHA, alpha, bestMove);
        return bestMove;
    }

    float negaMax(int depth, float alpha, float beta) throws InterruptedException {
        MoveGenerator mgen = new MoveGenerator(test);
        // hash the position
        long zhash = Zobrist.hash(test);
        if (depth <= 0 || mgen.moves.size() == 0 || mgen.gameOver != -1) {
            if (ttable.containsPosition(zhash)) {
                Position p0 = ttable.table[(int) Math.abs(zhash % ttable.getSize())];
                if (p0.depth >= depth) {
//					Zobrist.incrementHits();
                    if (p0.flag == Position.flagEXACT)
                        return p0.eval;
                    else if (p0.flag == Position.flagALPHA && p0.eval <= alpha)
                        return alpha;
                    else if (p0.flag == Position.flagBETA && p0.eval >= beta)
                        return beta;
//					Zobrist.decrementHits();
                }
            }
            if (mgen.moves.size() == 0 || mgen.gameOver != -1) {
                return evalMaterial(mgen, depth);
            } else {
                float score = quiescence(depth, alpha, beta);
                // replace this eval call with a quiescence search
                // float score = evalMaterial(mgen, depth);
                ttable.makePosition(zhash, depth, Position.flagEXACT, score, null);
                return score;
            }
        }
        // if the stored depth was >= remaining search depth, use that result
        if (ttable.containsPosition(zhash) && ttable.getDepth(zhash) >= depth) {
            Position p0 = ttable.table[(int) Math.abs(zhash % ttable.getSize())];
            if (p0.depth >= depth) {
//				Zobrist.incrementHits();
                if (p0.flag == Position.flagEXACT)
                    return p0.eval;
                else if (p0.flag == Position.flagALPHA && p0.eval <= alpha)
                    return alpha;
                else if (p0.flag == Position.flagBETA && p0.eval >= beta)
                    return beta;
//				Zobrist.decrementHits();
            }
        }
        mgen.moves = orderMovesByValue(mgen.moves, test);
        Move bestMove = null;
        for (Move m : mgen.moves) {
            if (Thread.interrupted()) // We've been interrupted: no more crunching.
                throw new InterruptedException();
            test.makeHistory(m);
            nodes++;
            float score = -negaMax(depth - 1, -beta, -alpha);
            test.unmakeMove();
            if (score >= beta) {
                ttable.makePosition(zhash, depth, Position.flagBETA, beta, null);
                return score;
            }
            if (score > alpha) {
                bestMove = m;
                alpha = score;
            }
        }
        ttable.makePosition(zhash, depth, Position.flagALPHA, alpha, bestMove);
        return alpha;
    }

    // TODO: verify that this helps
    float quiescence(int depth, float alpha, float beta) throws InterruptedException {
        MoveGenerator mgen = new MoveGenerator(test);
        float stand_pat = evalMaterial(mgen, depth);
        if (mgen.moves.size() == 0 || mgen.gameOver != -1) {
            return stand_pat;
        }
        if (stand_pat >= beta) return stand_pat;
        
        // Delta pruning: if a huge swing (> 1 queen)
        // is not enough to improve the position, give up
        final float DELTA = 9.75f;
        if (stand_pat < alpha - DELTA) return alpha;

        if (stand_pat > alpha) alpha = stand_pat;
        // hash the position
        long zhash = Zobrist.hash(test);
        // if the ttable stored the position at greater depth, use that result
        if (ttable.containsPosition(zhash) && ttable.getDepth(zhash) >= depth) {
            Position p0 = ttable.table[(int) Math.abs(zhash % ttable.getSize())];
            if (p0.depth >= depth) {
//				Zobrist.incrementHits();
                if (p0.flag == Position.flagEXACT)
                    return p0.eval;
                else if (p0.flag == Position.flagALPHA && p0.eval <= alpha)
                    return alpha;
                else if (p0.flag == Position.flagBETA && p0.eval >= beta)
                    return beta;
//				Zobrist.decrementHits();
            }
        }
        float score = stand_pat;
        mgen.moves = orderMovesByValue(mgen.moves, test);
        // make captures until no captures remain, then eval
        for (Move m : mgen.moves) {
            if (Thread.interrupted()) // We've been interrupted: no more crunching.
                throw new InterruptedException();
            if (test.isEmptySquare(m.end) && !m.isEnPassant)
                continue;
            test.makeHistory(m);
            nodes++;
            score = -quiescence(depth - 1, -beta, -alpha);
            test.unmakeMove();
            if (score >= beta) {
                ttable.makePosition(zhash, depth, Position.flagBETA, beta, null);
                return beta;
            }
            if (score > alpha) {
                alpha = score;
            }
        }
        ttable.makePosition(zhash, depth, Position.flagALPHA, alpha, null);
        return score;
    }

    // Orders moves based on the values of pieces on the given board
    // Checks capturing then pawn promotions then moving pieces by descending value
    ArrayList<Move> orderMovesByValue(ArrayList<Move> movein, Chess ch) {
        ArrayList<Move> moveout = new ArrayList<Move>();
        // store the indecies of sorted elements so they can be ignored later
        boolean map[] = new boolean[movein.size()];

        // check the ttable to see if this node has already been searched
        // even if the search was at a lower depth we check the previous best move first
        Move previousBest = ttable.getMove(Zobrist.hash(ch));
        if (previousBest != null)
            for (int i = 0; i < movein.size(); i++)
                if (movein.get(i).start == previousBest.start && movein.get(i).end == previousBest.end) {
//					Zobrist.incrementHits();
                    moveout.add(movein.get(i));
                    map[i] = true;
                }

        // moves by captures by captured type by descending value
        for (int type = Chess.QUEEN; type >= Chess.PAWN; type--)
            for (int i = 0; i < movein.size(); i++)
                if (!map[i] && (ch.board[movein.get(i).end] & Chess.TYPE_MASK) == type) {
                    moveout.add(movein.get(i));
                    map[i] = true;
                }
        // promotions!
        for (int i = 0; i < movein.size(); i++)
            if (!map[i] && movein.get(i).promoteType != 0) {
                moveout.add(movein.get(i));
                map[i] = true;
            }
        for (int type = Chess.QUEEN; type >= Chess.PAWN; type--)
            for (int i = 0; i < movein.size(); i++)
                if (!map[i] && (ch.board[movein.get(i).start] & Chess.TYPE_MASK) == type) {
                    map[i] = true;
                    moveout.add(movein.get(i));
                }
        for (int i = 0; i < movein.size(); i++)
            if (!map[i]) {
                moveout.add(movein.get(i));
            }
        assert moveout.size() == movein.size() : movein.size() - moveout.size();
        return moveout;
    }

}
*/
