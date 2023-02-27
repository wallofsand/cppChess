#include "Chess.h"
#include "Player.h"

U64 perft_root(Chess& ch, int8_t depth, bool initial_pos = false, int8_t log_depth = 1);
U64 perft(Chess& ch, int8_t depth, U64& nodes);
U64 eperft_root(Chess& ch, int8_t depth);
U64 eperft(Chess& ch, int8_t depth, U64& nodes);

const int8_t SIM_DEPTH = 3;

const std::string HELP_STRINGS[] = {
    "\nWelcome to Graham's C++ chess.\n"
    "Enter a move exactly as it is printed to play that move.\n"
    "Or enter one of the following commands:\n"
    "end: \tQuit the game.\n",
    "um x: \tUndo the last x moves.\n",
    "aim x: \tSearch to depth x and make a move. Allows depth of 1-9.\n",
    "\tSearches deeper than four ply may take extemely long.\n",
    "perft x: \tCount all moves at depth x. Allows any depth > -1.\n",
    "\tSearches deeper than six may take extremely long.\n",
    "eperft x: \tEval all positions at depth x. Allows any depth > -1.\n",
    "\tSearches deeper than six may take extremely long.\n",
    "help: \tDisplays this message.\n"
};

const std::string PERFT_RESULTS[] = {
    "1", "20", "400", "8902", "197281", "4865609", "119060324", "3195901860",
    "84998978956", "2439530234167", "69352859712417", "2097651003696806",
    "62854969236701747", "1981066775000396239", "61885021521585529237",
    "2015099950053364471960"
};

Player engine(1.0f);

int main()
{
    Compass();
    TTable();

    int human = -2;
    fmt::print("Welcome to Graham's C++ chess.\nWhich color will you play?\n0: white   1: black   2: sim game   -1: free play\n");
    while (human < -1 || human > 2)
        std::cin >> human;

    Chess ch(ch_cst::START_FEN);
    // Player low_mobility(0.80f), high_mobility(1.20f);
    // Player players[2] = { low_mobility, high_mobility };

    bool playing = true;
    std::string last_move = "ERROR";
    U64 nodes = 0;
    Timer game_timer;
    SearchLogger sim_log("sim_log", 0);

    // main game loop
    while (playing)
    {
        MoveGenerator mgen(ch);
        move moves[120] = {};
        mgen.gen_moves(moves);
        if (!moves[119] || ch.repetitions() > 2) playing = false;

        // print ui
        fmt::print("\n");
        ch.print_board(true);
        fmt::print("hash:  {:0>16x}\nwrites: {} hits: {} collisions: {}\n",
            ch.zhash,
            TTable::writes, TTable::hits, TTable::collisions);
        engine.eval(ch, 0, true);
        fmt::print("reps: {} nodes: {} n/s: {:0.0f}\n",
            ch.repetitions(), nodes, game_timer.elapsed() >= 0.1f ? nodes / game_timer.elapsed() : 0.0f);
        if (ch.history.size())
            fmt::print("{}{} {}\n", ((ch.history.size() - 1) / 2) + 1, ch.history.size() % 2 == 1 ? ". " : ".. ", last_move);
        if (!playing) break;
        if (!ch.black_to_move && human == 0 || ch.black_to_move && human == 1 || human == -1)
            for (uint8_t i = 0; i < moves[119]; i++)
                fmt::print("{} ", MoveGenerator::move_san(ch, moves[i]));
        fmt::print("\n{} ", ch.black_to_move ? "Black to move: " : "White to move: ");

        // get input
        game_timer.reset();
        std::string input = "";
        if (!ch.black_to_move && human == 0 || ch.black_to_move && human == 1 || human == -1)
            std::cin >> input;

        // if no input, get ai move
        if (!ch.black_to_move && human == 1 || ch.black_to_move && human == 0 || human == 2)
        {
            std::cout << "Pondering . . ." << std::endl;
            // move engine_move = players[ch.black_to_move].iterative_search(ch, SIM_DEPTH, nodes, false);
            move engine_move = engine.iterative_search(ch, SIM_DEPTH, nodes, false);
            last_move = MoveGenerator::move_san(ch, engine_move);
            ch.make_move(engine_move);
            if (human == 2)
                sim_log.write(last_move + " ");
        }

        // otherwise, handle input
        else if (input == "um" && ch.history.size() > 0)
        {
            int undos = 0;
            while (undos < 1 || undos > (int) ch.history.size())
                std::cin >> undos;
            ch.unmake_move(undos);
        }
        else if (input == "help" || input == "?")
            for (std::string tip : HELP_STRINGS)
                fmt::print("{}", tip);
        else if (input == "probe")
            fmt::print("{}\n", TTable::probe(ch.zhash).to_string());
        else if (input == "best")
        {
            Entry e = TTable::probe(ch.zhash);
            if (TTable::probe(ch.zhash).best)
                fmt::print("{}\n", MoveGenerator::move_san(ch, TTable::probe(ch.zhash).best));
            else
                fmt::print("No move found.\n");
        }
        else if (input == "null") // null move - skip your turn
            ch.black_to_move = !ch.black_to_move;
        else if (input == "aim")
        {
            int depth = 0;
            while (depth < 1 || depth > 9)
                std::cin >> depth;
            game_timer.reset();
            // move engine_move = players[ch.black_to_move].iterative_search(ch, depth, nodes, false);
            move engine_move = engine.iterative_search(ch, depth, nodes, false);
            last_move = MoveGenerator::move_san(ch, engine_move);
            ch.make_move(engine_move);
        }
        else if (input == "perft")
        {
            int depth = -1;
            while (depth < 0)
                std::cin >> depth;
            perft_root(ch, depth, !ch.history.size(), 1);
        }
        else if (input == "eperft")
        {
            int depth = -1;
            while (depth < 0)
                std::cin >> depth;
            eperft_root(ch, depth);
        }
        else if (input == "end")
            playing = false;
        else for (uint8_t i = 0; i < moves[119]; i++)
        {
            if (input != MoveGenerator::move_san(ch, moves[i]))
                continue;
            last_move = MoveGenerator::move_san(ch, moves[i]);
            ch.make_move(moves[i], true);
            break;
        }
    }

    MoveGenerator mate_gen(ch);
    fmt::print("\n");
    if (ch.repetitions() == 3)
        fmt::print("Draw by repitition!\n");
    else if (mate_gen.in_check)
        fmt::print("{} wins!\n", ch.black_to_move ? "White" : "Black");

    return 0;
}

SearchLogger perft_log("perft_log", 0);

/*
 * PERformance Test root method
 * @param chess the starting position to test
 * @param depth number of ply to search
 * @param initial_pos true if we are testing the initial position.
 *        if (initial_pos && depth < 16) we can print known target values.
 *        default: false
 * @param log_depth the depth of nodes to list in log file
 *        default: 0
 */
U64 perft_root(Chess& ch, int8_t depth, bool initial_pos, int8_t log_depth)
{
    log_depth = log_depth < depth ? log_depth : depth;
    perft_log.depth = depth - log_depth;
    U64 leaf_nodes = 0;
    U64 nodes = 0;
    if (depth > perft_log.depth)
        perft_log.write(fmt::format("Starting perft({}) at {}\n",
            depth, SearchLogger::time_to_string()));
    Timer perft_timer;
    MoveGenerator perft_gen(ch);
    move moves[120] = {};
    perft_gen.gen_moves(moves);

    // main test loop
    if (!depth)
        leaf_nodes = 1;
    else for (uint8_t mvidx = 0; mvidx < moves[119]; mvidx++)
    {
        move mv = moves[mvidx];
        std::cout << fmt::format("{}/{}:\t{} ", mvidx + 1, moves[119], Move::to_string(mv));
        if (depth > perft_log.depth)
            perft_log.buffer = fmt::format("{}/{}:\t{} ", mvidx + 1, moves[119], Move::to_string(mv));
        ch.make_move(mv);
        nodes++;
        U64 i = perft(ch, depth - 1, nodes);
        std::cout << i << std::endl;
        if (depth == 1 + perft_log.depth)
            perft_log.write(fmt::format("{}{}\n", perft_log.buffer, std::to_string(i)));
        leaf_nodes += i;
        ch.unmake_move(1);
    }
    double nodes_per_second = (double) nodes;
    if (perft_timer.elapsed() >= 0.001f)
        nodes_per_second = std::round(nodes_per_second / perft_timer.elapsed());

    // test finished, print results
    if (depth > perft_log.depth)
        perft_log.write(fmt::format("{}: search finished in {}s at {} nodes/s\n{} leaf nodes found at depth {}.\n",
            SearchLogger::time_to_string(), perft_timer.elapsed(), nodes_per_second, leaf_nodes, depth));
    std::cout << fmt::format("{}: search finished in {}s at {} nodes/s\n{} leaf nodes found at depth {}.\n",
            SearchLogger::time_to_string(), perft_timer.elapsed(), nodes_per_second, leaf_nodes, depth);

    // initial position tested, verify results
    if (initial_pos)
    {
        if (depth > perft_log.depth)
        {
            perft_log.write(fmt::format("{} nodes expected. ", PERFT_RESULTS[depth]));
            if (leaf_nodes != std::stoll(PERFT_RESULTS[depth]))
                perft_log.write("Uh oh!\n");
            else
                perft_log.write("Nice!\n");
        }
        std::cout << PERFT_RESULTS[depth] << " nodes expected. ";
        if (leaf_nodes != std::stoll(PERFT_RESULTS[depth]))
            std::cout << "Uh oh!";
        else
            std::cout << "Nice!";
        std::cout << std::endl;
    }
    if (depth > perft_log.depth)
        perft_log.write("\n");
    return leaf_nodes;
}

/*
 * PERformance Test recursion method
 * @param ch the current position to search
 * @param depth number of ply remaining in the search
 */
U64 perft(Chess& ch, int8_t depth, U64& nodes)
{
    if (!depth)
        return 1;
    U64 leaf_nodes = 0;
    MoveGenerator perft_gen(ch);
    move moves[120] = {};
    perft_gen.gen_moves(moves);
    for (uint8_t mvidx = 0; mvidx < moves[119]; mvidx++)
    {
        if (depth > perft_log.depth)
            perft_log.buffer += fmt::format(" {}", MoveGenerator::move_san(ch, moves[mvidx]));
        ch.make_move(moves[mvidx]);
        nodes++;
        U64 i = perft(ch, depth - 1, nodes);
        leaf_nodes += i;
        ch.unmake_move(1);
        if (depth == 1 + perft_log.depth)
            perft_log.write(fmt::format("{} {}\n", perft_log.buffer, std::to_string(i)));
        if (depth > perft_log.depth)
            perft_log.buffer.erase(perft_log.buffer.find_last_of(' '), perft_log.buffer.length());
    }
    return leaf_nodes;
}

/*
 * Evaluation PERformance Test root method
 * @param chess the starting position to test
 * @param depth number of ply to search
 * @param initial_pos true if we are testing the initial position.
 *        if (initial_pos && depth < 16) we can print known target values.
 *        default: false
 * @param log_depth the depth of nodes to list in log file
 *        default: 0
 */
U64 eperft_root(Chess& ch, int8_t depth)
{
    // run normal perft for baseline
    perft_root(ch, depth, false, 0);

    std::cout << fmt::format("{}: begin eval test...\n", SearchLogger::time_to_string());

    U64 leaf_nodes = 0;
    U64 nodes = 0;
    Timer eperft_timer;
    MoveGenerator eperft_gen(ch);
    move moves[120] = {};
    eperft_gen.gen_moves(moves);

    // main test loop
    if (!depth)
        leaf_nodes = 1;
    else for (uint8_t mvidx = 0; mvidx < moves[119]; mvidx++)
    {
        ch.make_move(moves[mvidx]);
        nodes++;
        U64 i = eperft(ch, depth - 1, nodes);
        leaf_nodes += i;
        ch.unmake_move(1);
    }
    double nodes_per_second = (double) nodes;
    if (eperft_timer.elapsed() >= 0.001f)
        nodes_per_second = nodes_per_second / eperft_timer.elapsed();

    // test finished, print results
    std::cout << fmt::format("{}: eval test finished in {}s at {} nodes/s\n{} leaf nodes found at depth {}.\n",
            SearchLogger::time_to_string(), eperft_timer.elapsed(), nodes_per_second, leaf_nodes, depth);

    return leaf_nodes;
}

/*
 * Evaltuation PERformance Test recursion method
 * @param ch the current position to search
 * @param depth number of ply remaining in the search
 */
U64 eperft(Chess& ch, int8_t depth, U64& nodes)
{
    if (!depth)
    {
        engine.eval(ch, depth);
        return 1;
    }
    U64 leaf_nodes = 0;
    MoveGenerator eperft_gen(ch);
    move moves[120] = {};
    eperft_gen.gen_moves(moves);
    for (uint8_t mvidx = 0; mvidx < moves[119]; mvidx++)
    {
        ch.make_move(moves[mvidx]);
        nodes++;
        U64 i = eperft(ch, depth - 1, nodes);
        leaf_nodes += i;
        ch.unmake_move(1);
    }
    return leaf_nodes;
}
