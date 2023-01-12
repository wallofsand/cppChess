#include "Player.h"
#include <iostream>

using namespace ch_cst;

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
    Compass();
    TTable();
    Chess chess;
    Player pl;

    // chess.make_move(Move(Square::c2, Square::c3));
    // chess.make_move(Move(Square::a7, Square::a6));
    // chess.make_move(Move(Square::d1, Square::a4));

    bool playing = true;
    std::string last_move = "";
    MoveGenerator mgen(chess);
    U64 nodes = 0;
    while (playing)
    {
        std::vector<Move> move_list = mgen.gen_moves();
        fmt::print("\n");
        chess.print_board(true);
        fmt::print("hash: {} w: {} h: {} c: {}\neval: {} n/s: {}\n",
            chess.hash(), TTable::writes, TTable::hits, TTable::clashes, pl.eval(chess), nodes);
        if (chess.ply_counter)
            fmt::print("{}{} {}\n", ((chess.ply_counter - 1) / 2) + 1, chess.ply_counter % 2 == 1 ? ". " : ".. ", last_move);
        for (Move mv : move_list)
            fmt::print("{} ", mgen.move_san(mv));
        fmt::print("\n{} ", chess.aci ? "Black to move:" :"White to move: ");

        std::string mv_str;
        std::cin >> mv_str;

        if (mv_str.substr(0, 2) == "um" && chess.ply_counter > 0)
        {
            int undos = 0;
            while (undos < 1 || undos > chess.ply_counter)
                std::cin >> undos;
            chess.unmake_move(undos);
        }
        else if (mv_str.length() == 1)
        {
            int depth = (int) mv_str[0] - 48;
            if (depth < 0 || depth > 9) continue;
            SearchLogger sl("search_log", 1);
            Move engine_move = pl.get_move(chess, sl, depth, nodes, false);
            last_move = mgen.move_san(engine_move);
            chess.make_move(engine_move);
        }
        else if (mv_str == "aim")
        {
            int depth = 0;
            while (depth < 1 || depth > 6)
                std::cin >> depth;
            SearchLogger sl("search_log", 1);
            Move engine_move = pl.get_move(chess, sl, depth, nodes, false);
            last_move = mgen.move_san(engine_move);
            chess.make_move(engine_move);
        }
        else if (mv_str == "perft")
        {
            int depth = 0;
            while (depth < 1)
                std::cin >> depth;
            perft_root(chess, depth, chess.ply_counter == 0, 1);
        }
        else if (mv_str == "test")
        {
            mgen.gen_moves(true);
        }
        else if (mv_str == "end") playing = false;
        else for (Move mv : move_list)
            if (mv_str == mgen.move_san(mv))
            {
                last_move = mgen.move_san(mv);
                chess.make_move(mv);
                break;
            }
        if (mgen.is_game_over(false)) playing = false;
        if (!playing) chess.print_board(true);
    }

    // for (int idx = 0; idx < TTable::DEFAULT_SIZE; idx++)
    //     if (TTable::read(idx).key && TTable::read(idx).flag != 1)
    //         fmt::print("{}: {}\n", idx, TTable::read(idx).to_string());

    return 0;
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
U64 perft_root(Chess& ch, int depth, bool initial_pos, int log_depth)
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
    Timer perft_timer;

    // main test loop
    MoveGenerator perft_gen(ch);
    std::vector<Move> moves = perft_gen.gen_moves();
    for (int mvidx = 0; mvidx < (int) moves.size(); mvidx++)
    {
        Move mv = moves.at(mvidx);
        std::cout << fmt::format("{}/{}:\t{} ", mvidx + 1, moves.size(), mv.to_string());
        if (depth > perft_log.depth)
        {
            perft_log.buffer = fmt::format("{}/{}:\t{} ", mvidx + 1, moves.size(), mv.to_string());
        }
        ch.make_move(mv);
        U64 i = perft(ch, depth - 1, perft_log);
        std::cout << i << std::endl;
        if (depth == 1 + perft_log.depth)
            perft_log.write(perft_log.buffer + std::to_string(i) + "\n");
        nodes += i;
        ch.unmake_move(1);
    }
    double nodes_per_second = (double) nodes;
    if (perft_timer.elapsed() >= 1)
        nodes_per_second = nodes_per_second / perft_timer.elapsed();

    // test finished, print results
    if (depth > perft_log.depth)
        perft_log.write(fmt::format("{}: finished in {}s at {} nodes/s\n{} moves found\n",
            SearchLogger::time_to_string(), perft_timer.elapsed(), nodes_per_second, nodes));
    std::cout << fmt::format("{}: finished in {}s at {} nodes/s\n{} moves found\n",
            SearchLogger::time_to_string(), perft_timer.elapsed(), nodes_per_second, nodes);

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
            perft_log.buffer += fmt::format(" {}", perft_gen.move_san(mv));
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
