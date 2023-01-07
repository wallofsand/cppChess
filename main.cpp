#include "Chess.h"
#include "Move.h"
#include "MoveGenerator.h"
#include <iostream>

using namespace ch_cst;

// U64 perft_root(Chess& chess, int depth, bool initial_position, int log_depth = 0);
// U64 perft(MoveGenerator& perft_gen, Chess& chess, int depth, SearchLogger& perft_log);
void print_board(Chess ch, bool fmt = true);
void print_U64(U64 bb, bool fmt = true);
U64 perft_root(Chess& ch, int depth, bool initial_pos, int log_depth);
U64 perft(Chess& ch, int depth, SearchLogger& perft_log);

const std::string perft_results[] = {
    "1", "20", "400", "8902", "197281", "4865609", "119060324", "3195901860",
    "84998978956", "2439530234167", "69352859712417", "2097651003696806",
    "62854969236701747", "1981066775000396239", "61885021521585529237",
    "2015099950053364471960"
};

int main()
{
    Compass::init_compass();
    Chess chess;

    perft_root(chess, 3, true, 1);
    perft_root(chess, 3, true, 1);
    perft_root(chess, 3, true, 1);
    perft_root(chess, 3, true, 1);
    perft_root(chess, 3, true, 1);

    bool playing = false;
    MoveGenerator mgen(chess);
    while (playing)
    {
        mgen.set_chess(chess);
        std::vector<Move> move_list = mgen.gen_moves();
        chess.print_board(true);
        std::string mv_str;
        for (Move mv : move_list)
            std::cout << chess.move_fen(mv) << " ";
        std::cout << std::endl << (chess.aci ? "Black to move: " : "White to move: ");
        std::cin >> mv_str;
        if (mv_str.substr(0, 2) == "um" && (std::stoi(mv_str.substr(2)) <= chess.ply_counter))
            chess.unmake_move(std::stoi(mv_str.substr(2)));
        else if (mv_str == "test")
        {
            print_U64(Bitboard::SoEa_attacks(chess.bb_queens & chess.bb_black, ~chess.bb_occ), true);
        }
        else for (Move mv : move_list)
            if (mv_str == chess.move_fen(mv))
                chess.make_move(mv);
    }
    return 0;
}

void print_U64(U64 bb, bool fmt)
{
    Bitboard::print_binary_string(Bitboard::build_binary_string(bb), fmt);
}

/*
 * Performance test root method
 * @param chess the starting position to test
 * @param depth number of ply to search
 * @param initial_pos true if we are testing the initial position.
 *        if (initial_pos && depth < 16) we can print known target values.
 *        default: false
 * @param log_depth the depth of nodes to list in log file
 *        default: 0
 */
U64 perft_root(Chess &ch, int depth, bool initial_pos, int log_depth)
{
    if (!depth)
        return 1;
    log_depth = std::min(log_depth, depth);
    SearchLogger perft_log("perft_log", depth - log_depth);
    U64 nodes = 0;
    if (depth > perft_log.depth)
    {
        perft_log.write(fmt::format("Starting perft({}) at {}\n",
            depth, SearchLogger::time_to_string()));
    }
    std::cout << "Starting perft(" << depth << ") at "
            << SearchLogger::time_to_string() << std::endl;
    Timer perft_timer;

    // main test loop
    MoveGenerator perft_gen(ch);
    std::vector<Move> moves = perft_gen.gen_moves();
    for (int mvidx = 0; mvidx < (int) moves.size(); mvidx++)
    {
        Move mv = moves.at(mvidx);
        std::cout << fmt::format("{}/{}:\t{}\t\t", mvidx + 1, moves.size(), ch.move_fen(mv));
        if (depth > perft_log.depth)
        {
            perft_log.buffer = fmt::format("{}/{}:\t{} ", mvidx + 1, moves.size(), ch.move_fen(mv));
        }
        ch.make_move(mv);
        U64 i = perft(ch, depth - 1, perft_log);
        std::cout << i << std::endl;
        if (depth == 1 + perft_log.depth)
            perft_log.write(perft_log.buffer + std::to_string(i) + "\n");
        nodes += i;
        ch.unmake_move(1);
    }
    U64 nodes_per_second = nodes;
    if (perft_timer.elapsed() > 0.001)
        nodes_per_second = nodes_per_second / (U64) (perft_timer.elapsed() * 1000);

    // test finished, print results
    if (depth > perft_log.depth)
        perft_log.write(fmt::format("{}: {} moves found in {}s at {} nodes/s.\n",
            SearchLogger::time_to_string(), nodes, perft_timer.elapsed(), nodes_per_second));
    std::cout << fmt::format("{}: {} moves found in {}s at {} nodes/s.\n",
            SearchLogger::time_to_string(), nodes, perft_timer.elapsed(), nodes_per_second);

    // initial position tested, verify results
    if (initial_pos)
    {
        if (depth > perft_log.depth)
        {
            perft_log.write(fmt::format("{} moves expected. ", perft_results[depth]));
            if (nodes != std::stoll(perft_results[depth]))
                perft_log.write("Uh oh!\n");
            else
                perft_log.write("Nice!\n");
        }
        std::cout << perft_results[depth] << " moves expected. ";
        if (nodes != std::stoll(perft_results[depth]))
            std::cout << "Uh oh!";
        else
            std::cout << "Nice!";
        std::cout << std::endl;
    }
    if (depth > perft_log.depth)
        perft_log.write("\n");
    return nodes;
}

/*
 * Performance test recursion method
 * @param chess the current position to search
 * @param depth number of ply remaining in the search
 */
U64 perft(Chess& ch, int depth, SearchLogger& perft_log)
{
    if (!depth)
        return 1;
    U64 nodes = 0;
    MoveGenerator perft_gen(ch);
    std::vector<Move> moves = perft_gen.gen_moves();
    for (Move mv : moves)
    {
        // std::cout << mv << std::endl;
        if (depth > perft_log.depth)
            perft_log.buffer += fmt::format(" {}", ch.move_fen(mv));
        ch.make_move(mv);
        U64 i = perft(ch, depth - 1, perft_log);
        nodes += i;
        ch.unmake_move(1);
        if (depth == 1 + perft_log.depth)
            perft_log.write(perft_log.buffer + " " + std::to_string(i) + "\n");
        if (depth > perft_log.depth)
            perft_log.buffer.erase(perft_log.buffer.find_last_of(' '), perft_log.buffer.length());
    }
    return nodes;
}
