#include "Chess.h"
#include "Player.h"

U64 perft_root(int depth, int log_depth = 1);
U64 perft(int depth, U64& nodes);
U64 eperft_root(int depth);
U64 eperft(int depth, U64& nodes);

const int SIM_DEPTH = 4;

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

const std::string PERFT_RESULTS[16] = {
    "1", "20", "400", "8902", "197281", "4865609", "119060324", "3195901860",
    "84998978956", "2439530234167", "69352859712417", "2097651003696806",
    "62854969236701747", "1981066775000396239", "61885021521585529237",
    "2015099950053364471960"
};

Player engine(1.0f);

int main(int arg0, char** args)
{
    Compass();
    TTable();

    enum human_index {
        PLAY_WHITE,
        PLAY_BLACK,
        PLAY_SIM,
        PLAY_FREE,
        PLAY_STOP
    } human;
    fmt::print("Welcome to Graham's C++ chess.\nWhich color will you play?\n   0: white      1: black\n   2: sim game   3: free play\n");
    int in = PLAY_STOP;
    while (in < PLAY_WHITE || in >= PLAY_STOP)
        std::cin >> in;
    human = human_index(in);

    // Chess::stack.top->pos = new Chess (ch_cst::TEST_FEN);
    Chess::stack.top->pos = new Chess (ch_cst::START_FEN);

    // Player low_mobility(0.80f), high_mobility(1.20f);
    // Player players[2] = { low_mobility, high_mobility };

    bool playing = true;
    std::string last_move = "ERROR";
    Timer game_timer;
    SearchLogger sim_log("sim_log", 0, (fmt::file::WRONLY | fmt::file::CREATE | fmt::file::APPEND));

    U64 nodes = 0;

    // main game loop
    while (playing)
    {
        Chess& ch = *Chess::state();
        MoveGenerator mgen(ch);
        move moves[MAXMOVES] = {};
        mgen.gen_moves(moves);

        // print ui
        fmt::print("\n");
        ch.print_board(true);
        fmt::print("fen: {}\nhash: {:0>16X}\nwrites: {} hits: {} fill: %{:2.2f}\n",
            ch.fen(), ch.zhash, TTable::writes, TTable::hits, TTable::fill_ratio() * 100);
        engine.eval(0, true);
        fmt::print("nodes: {:<10d} n/s: {:0.3f} time: {:0.3f}s\n",
                nodes, game_timer.elapsed() >= 0.01f ? nodes / game_timer.elapsed() : 0.0f, game_timer.elapsed());
        if (Chess::stack.top->next)
            fmt::print("{}{} {}\n", Chess::stack.top->next->pos->fullmoves, ch.black_to_move ? ". " : ".. ", last_move);
        fmt::print("reps: {} halfmoves: {}\n", ch.repetitions(), ch.halfmoves);
        if (!playing) break;
        if (!ch.black_to_move && human == PLAY_WHITE || ch.black_to_move && human == PLAY_BLACK || human == PLAY_FREE)
            for (int i = 0; i < moves[MAXMOVES - 1]; i++)
                fmt::print("{} ", MoveGenerator::move_san(moves[i]));
        fmt::print("\n{} to move: ", ch.black_to_move ? "Black" : "White");

        nodes = 0;
        game_timer.reset();
        std::string input = "";

        if (human == PLAY_SIM && mgen.is_game_over(false))
            break;

        // get input
        if (!ch.black_to_move && human == PLAY_WHITE
                || ch.black_to_move && human == PLAY_BLACK
                || human == PLAY_FREE)
            std::cin >> input;
        else
        {
            // get computer player input
            std::cout << "Pondering . . ." << std::endl;
            // move engine_move = players[ch.black_to_move].iterative_search(ch, SIM_DEPTH, nodes, false);
            move engine_move = engine.iterative_search(SIM_DEPTH, nodes, false);
            last_move = MoveGenerator::move_san(engine_move);
            Chess::push_move(engine_move);
            if (human == PLAY_SIM) {
                if (!ch.black_to_move) sim_log.write(std::to_string(ch.fullmoves) + ". ");
                sim_log.write(last_move + " ");
            }
            continue;
        }

        // otherwise, handle input
        if (input == "um")
        {
            unsigned int undos = 0;
            while (undos < 1)
                std::cin >> undos;
            Chess::unmake_move(undos);
        }
        else if (input == "fen")
            fmt::print("{}\n", ch.fen());
        else if (input == "help" || input == "?")
            for (std::string tip : HELP_STRINGS)
                fmt::print("{}", tip);
        else if (input == "probe")
            fmt::print("{}\n", TTable::probe(ch.zhash).to_string());
        else if (input == "best")
        {
            Entry e = TTable::probe(ch.zhash);
            if (TTable::probe(ch.zhash).best)
                fmt::print("{}\n", MoveGenerator::move_san(TTable::probe(ch.zhash).best));
            else
                fmt::print("No move found.\n");
        }
        else if (input == "null" && human == PLAY_FREE) // null move - skip your turn. highly illegal!
        {
            ch.black_to_move = !ch.black_to_move;
            ch.zhash ^= TTable::is_black_turn;
        }
        else if (input == "aim")
        {
            int depth = 0;
            while (depth < 1 || depth > 9)
                std::cin >> depth;
            game_timer.reset();
            // move engine_move = players[ch.black_to_move].iterative_search(ch, depth, nodes, false);
            move engine_move = engine.iterative_search(depth, nodes, false);
            last_move = MoveGenerator::move_san(engine_move);
            Chess::push_move(engine_move);
        }
        else if (input == "perft")
        {
            int depth = -1;
            while (depth < 0)
                std::cin >> depth;
            perft_root(depth, 1);
        }
        else if (input == "eperft")
        {
            int depth = -1;
            while (depth < 0)
                std::cin >> depth;
            eperft_root(depth);
        }
        else if (input == "end")
            playing = false;
        else for (int i = 0; i < moves[MAXMOVES - 1]; i++)
        {
            if (input != MoveGenerator::move_san(moves[i]))
                continue;
            last_move = MoveGenerator::move_san(moves[i]);
            Chess::push_move(moves[i], true);
            break;
        }
    }

    MoveGenerator mate_gen(Chess::state());
    fmt::print("\n");
    if (Chess::state()->repetitions() == 3)
        fmt::print("Draw by repitition!\n");
    else if (mate_gen.in_check)
        fmt::print("{} wins!\n", Chess::state()->black_to_move ? "White" : "Black");

    return 0;
}

SearchLogger perft_log("perft_log", 0);

/*
 * PERformance Test root method
 * @param chess the starting position to test
 * @param depth number of ply to search
 * @param log_depth the depth of nodes to list in log file
 *        default: 0
 */
U64 perft_root(int depth, int log_depth)
{
    Chess ch = *Chess::state();
    log_depth = log_depth < depth ? log_depth : depth;
    perft_log.depth = depth - log_depth;
    U64 leaf_nodes = 0;
    U64 nodes = 0;
    if (depth > perft_log.depth)
        perft_log.write(fmt::format("Starting perft({}) at {}\n",
            depth, SearchLogger::time_to_string()));
    Timer perft_timer;
    MoveGenerator perft_gen(ch);
    move moves[MAXMOVES] = {};
    perft_gen.gen_moves(moves);

    // main test loop
    if (!depth)
        leaf_nodes = 1;
    else for (int mvidx = 0; mvidx < moves[MAXMOVES - 1]; mvidx++)
    {
        move mv = moves[mvidx];
        std::cout << fmt::format("{}/{}:\t{} ", mvidx + 1, moves[MAXMOVES - 1], Move::to_string(mv));
        if (depth > perft_log.depth)
            perft_log.buffer = fmt::format("{}/{}:\t{} ", mvidx + 1, moves[MAXMOVES - 1], Move::to_string(mv));
        Chess::push_move(mv);
        nodes++;
        U64 i = perft(depth - 1, nodes);
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
    if (ch.fen() == ch_cst::START_FEN)
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
U64 perft(int depth, U64& nodes)
{
    if (!depth)
        return 1;
    Chess ch = *Chess::state();
    U64 leaf_nodes = 0;
    MoveGenerator perft_gen(ch);
    move moves[MAXMOVES] = {};
    perft_gen.gen_moves(moves);
    for (int mvidx = 0; mvidx < moves[MAXMOVES - 1]; mvidx++)
    {
        if (depth > perft_log.depth)
            perft_log.buffer += fmt::format(" {}", MoveGenerator::move_san(moves[mvidx]));
        Chess::push_move(moves[mvidx]);
        nodes++;
        U64 i = perft(depth - 1, nodes);
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
U64 eperft_root(int depth)
{
    // run normal perft for baseline
    perft_root(depth, 0);

    Chess ch = *Chess::state();
    std::cout << fmt::format("{}: begin eval test...\n", SearchLogger::time_to_string());

    U64 leaf_nodes = 0;
    U64 nodes = 0;
    Timer eperft_timer;
    MoveGenerator eperft_gen(ch);
    move moves[MAXMOVES] = {};
    eperft_gen.gen_moves(moves);

    // main test loop
    if (!depth)
        leaf_nodes = 1;
    else for (int mvidx = 0; mvidx < moves[MAXMOVES - 1]; mvidx++)
    {
        Chess::push_move(moves[mvidx]);
        nodes++;
        U64 i = eperft(depth - 1, nodes);
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
U64 eperft(int depth, U64& nodes)
{
    Chess ch = *Chess::state();
    if (!depth)
    {
        engine.eval(depth);
        return 1;
    }
    U64 leaf_nodes = 0;
    MoveGenerator eperft_gen(ch);
    move moves[MAXMOVES] = {};
    eperft_gen.gen_moves(moves);
    for (int mvidx = 0; mvidx < moves[MAXMOVES - 1]; mvidx++)
    {
        Chess::push_move(moves[mvidx]);
        nodes++;
        U64 i = eperft(depth - 1, nodes);
        leaf_nodes += i;
        ch.unmake_move(1);
    }
    return leaf_nodes;
}
