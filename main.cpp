#include "Chess.h"
#include "Player.h"

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
    Chess ch;
    Player engine(0.0f), low_mobility(-0.02f), high_mobility(0.02f);
    Player players[2] = { low_mobility, high_mobility };

    int human = -2;
    fmt::print("Which color will you play?\n0: white   1: black   2: sim game   -1: free play\n");
    while (human < -1 || human > 2)
    {
        std::cin >> human;
    }

    bool playing = true;
    std::string last_move = "ERROR";
    U64 nodes = 0;
    Timer game_timer;

    // main game loop
    while (playing)
    {
        MoveGenerator mgen(ch);
        std::vector<Move> move_list = mgen.gen_moves();

        // print ui
        if (ch.aci == human || human == -1 || human == 2)
        {
            fmt::print("\n");
            ch.print_board(true);
            fmt::print("hash: {:0>16x}\nwrites: {} hits: {} collisions: {} fill: {:0.2f}\n",
                ch.zhash,
                TTable::writes, TTable::hits, TTable::collisions, TTable::fill_ratio());
            engine.eval(ch, move_list, 0, true);
            fmt::print("reps: {} nodes: {} n/s: {:0.0f}\n",
                ch.repetitions(), nodes, game_timer.elapsed() >= 0.1f ? nodes / game_timer.elapsed() : 0.0f);
            if (ch.ply_counter)
                fmt::print("{}{} {}\n", ((ch.ply_counter - 1) / 2) + 1, ch.ply_counter % 2 == 1 ? ". " : ".. ", last_move);
            for (Move mv : move_list)
                fmt::print("{} ", mgen.move_san(mv));
            fmt::print("\n{} ", ch.aci ? "Black to move: " : "White to move: ");
        }
        // else if (ch.ply_counter)
        // {
        //     if (ch.ply_counter % 2 == 1) // last move was white
        //         fmt::print("{}.\t{}\t", ((ch.ply_counter - 1) / 2) + 1, last_move);
        //     else // last move was black
        //         fmt::print("{}.. {}\n", ((ch.ply_counter - 1) / 2) + 1, last_move);
        // }

        // get input
        game_timer.reset();
        std::string str = "2";
        if (ch.aci == human || human == -1) std::cin >> str;
        // handle input
        if (str.substr(0, 2) == "um" && ch.ply_counter > 0)
        {
            int undos = 0;
            while (undos < 1 || undos > ch.ply_counter)
                std::cin >> undos;
            ch.unmake_move(undos);
        }
        else if (str.length() == 1)
        {
            int depth = (int) str[0] - 48;
            if (depth < 1 || depth > 9) continue;
            game_timer.reset();
            Move engine_move = players[ch.aci].iterative_search(ch, depth, nodes, false);
            last_move = mgen.move_san(engine_move);
            ch.make_move(engine_move);
        }
        else if (str == "aim")
        {
            int depth = 0;
            while (depth < 1 || depth > 9)
                std::cin >> depth;
            game_timer.reset();
            Move engine_move = players[ch.aci].iterative_search(ch, depth, nodes, false);
            last_move = mgen.move_san(engine_move);
            ch.make_move(engine_move);
        }
        else if (str == "perft")
        {
            int depth = 0;
            while (depth < 1)
                std::cin >> depth;
            perft_root(ch, depth, ch.ply_counter == 0, 1);
        }
        else if (str == "reps")
            fmt::print("{}", ch.repetitions());
        else if (str == "test")
        {
            engine.iterative_search(ch, 1, nodes, true);
        }
        else if (str == "end")
            playing = false;
        else for (Move mv : move_list)
            if (str == mgen.move_san(mv))
            {
                last_move = mgen.move_san(mv);
                ch.make_move(mv, true );
                break;
            }
        if (mgen.is_game_over(false)) playing = false;
        if (!playing)
        {
            fmt::print("\n");
            ch.print_board(true);
        }
    }

    // for (int idx = 0; idx < TTable::DEFAULT_SIZE; idx++)
    //     if (TTable::bin[idx]) fmt::print("{}: {}, ", idx, TTable::bin[idx]);

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
 * @param ch the current position to search
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
