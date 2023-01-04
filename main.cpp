#include "Chess.h"
#include "MoveGenerator.h"
#include "SearchLogger.h"
#include <iostream>

using namespace ch_cst;

U64 perft_root(Chess *chess, int depth, int log_depth, bool initial_pos);
U64 perft(MoveGenerator *perft_gen, Chess *chess, int depth);
void print_board(Chess ch, bool fmt = true);
void print_U64(U64 bb, bool fmt = true);

int main()
{
    Compass::init_compass();
    Chess ch0;
    MoveGenerator mgen(ch0);

    std::cout << SearchLogger::date_to_string() << std::endl;

    // perft_root(&ch0, 1, 1, true);

    return 0;
}

const std::string perft_results[] = {
    "1", "20", "400", "8902", "197281", "4865609", "119060324", "3195901860",
    "84998978956", "2439530234167", "69352859712417", "2097651003696806",
    "62854969236701747", "1981066775000396239", "61885021521585529237",
    "2015099950053364471960"
};

/*
 * Performance test root method
 * @param chess the starting position to test
 * @param depth number of ply to search
 * @param log_depth the depth of nodes to list in log file
 *        default: 0
 * @param initial_pos true if we are testing the initial position.
 *        if (initial_pos && depth < 16) we can print known target values.
 *        default: false
 */
U64 perft_root(Chess *chess, int depth, int log_depth, bool initial_pos)
{
    if (depth == 0)
        return 1;
    Timer t;
    U64 nodes = 0;
    std::cout << "Starting perft(" << depth << "):" << std::endl;
    MoveGenerator perft_gen(*chess);
    std::vector<Move> moves = perft_gen.gen_moves();
    for (int mvidx = 0; mvidx < moves.size(); mvidx++)
    {
        Move mv = moves.at(mvidx);
        if (log_depth)
            std::cout << mvidx + 1 << "/" << moves.size() << ": (" << mv << ") ";
        (*chess).make_move(mv);
        U64 i = perft(&perft_gen, chess, depth - 1);
        if (log_depth)
            std::cout << i << std::endl;
        nodes += i;
        (*chess).unmake_move(1);
    }
    std::cout << nodes << " moves found in " << t.elapsed() << " seconds." << std::endl;
    if (initial_pos)
    {
        std::cout << perft_results[depth] << " moves expected. ";
        if (nodes != std::stoll(perft_results[depth]))
            std::cout << "Uh oh!";
        else
            std::cout << "Nice!";
        std::cout << std::endl;
    }
    // print_board(chess);
    return nodes;
}

/*
 * Performance test recursion method
 * @param chess the current position to search
 * @param depth number of ply remaining in the search
 */
U64 perft(MoveGenerator *perft_gen, Chess *chess, int depth)
{
    if (depth == 0)
        return 1;
    U64 nodes = 0;
    (*perft_gen).set_chess(*chess);
    std::vector<Move> moves = (*perft_gen).gen_moves();
    for (Move mv : moves) {
        (*chess).make_move(mv);
        nodes += perft(perft_gen, chess, depth - 1);
        (*chess).unmake_move(1);
    }
    return nodes;
}

void print_board(Chess ch, bool fmt)
{
    std::string board = "";
    for (int sq = 0; sq < 64; sq++)
    {
        for (int color = 0; color < 2; color++)
        {
            for (int piece = ch_cst::PAWN; piece <= ch_cst::KING; piece++)
            {
                if (Bitboard::contains_square(*ch.bb_by_piece[piece] & *ch.bb_by_color[color], sq))
                {
                    board += piece_char[piece | (color << 3)];
                }
            }
        }
        if (board.length() > sq)
            continue;
        board += '.';
    }
    Bitboard::print_binary_string(board, fmt);
}

void print_U64(U64 bb, bool fmt)
{
    Bitboard::print_binary_string(Bitboard::build_binary_string(bb), fmt);
}
