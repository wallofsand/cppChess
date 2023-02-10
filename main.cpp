#include "Chess.h"
#include "Player.h"

// fmt::print("zhash:  {:0>16x}\nhash(): {:0>16x}\n", ch.zhash, ch.hash());

U64 perft_root(Chess& ch, int depth, bool initial_pos, int log_depth);
U64 perft(Chess& ch, int depth, SearchLogger& perft_log);

const int SIM_DEPTH = 3;

const std::string HELP_STRINGS[] = {
    "\nWelcome to Graham's C++ chess.\n"
    "Enter a move exactly as it is printed to play that move.\n"
    "Or enter one of the following commands:\n"
    "end: \tQuit the game.\n",
    "um x: \tUndo the last x moves.\n",
    "aim x: \tSearch to depth x and make a move. Allows depth of 1-9.\n",
    "\tSearches deeper than four ply may take extemely long.\n",
    "perft x: \tCount all moves to depth x.\n",
    "\tSearches deeper than six may take extremely long.\n",
    "help: \tDisplays this message.\n"
};

const std::string PERFT_RESULTS[] = {
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
    // Player players[2] = { low_mobility, high_mobility };

    // e4 e5 d4 Bb4+ Nc3 d5 Bb5+ Nc6 Be3 Nf6 Bg5 Qd6 Bxf6 Bxc3+ bxc3 gxf6 Qf3 a6 Bxc6+ bxc6 O-O-O Qa3+ Kd2 exd4 cxd4 Qb4+ Kc1 Qd6 Ne2 Rb8 Nf4 Be6 Nh5 f5 Ng7+ Kd7 exf5 Qb4 fxe6+ fxe6 Qf7+ Qe7 Qxe7+ Kxe7 Rhe1 Kf6 Nxe6 Rhc8 Nc5 Re8 Nd7+ Kg5 Nxb8 Rxb8 Re5+ Kf6 h4 Rg8 Rg1 Rg4 Rh5 Kg6 Rg5+ Rxg5 hxg5 Kxg5 Re1 a5 Re7 a4 Rxc7 c5 Rxc5 a3 Rc3 Kf5 Rxa3 h5 Re3 h4 Re5+ Kf6 Rxd5 Ke6 Rh5 Kd6 a3 Ke6 a4 Kd6 a5 Ke6 a6 Kd6 a7 Ke6 a8=Q Kf6 Qc6+ Kf7 d5 h3 Qe6+ Kf8 Rxh3 Kg7 Rf3 Kh7 d6 Kg7 d7 Kh7 d8=Q Kg7 Qd4+ Kh7 Qd4d7+ Kh8 Qg7+ Kxg7 Rf4 Kh7 Rb4 Kg7 Rb8 Kh7 Rb7+ Kh8 Qe5+ Kg8 Qd6 Kh8 Qe5+ Kg8 Qd6 Kh8 Qd4+ Kg8 Qh8+ Kxh8 Rd7 Kg8 Rb7 Kf8 Kd2 Ke8 Kd3 Kd8 Kd4 Kc8 Re7 Kd8 Re1 Kc7 Kd5 Kc8 c4 Kc7 c5 Kc8 c6 Kd8 Kd6 Kc8 c7 Kb7 Kd7 Kb6 c8=Q Ka5 Qb8 Ka4 Qb6 Ka3 Qb1 Ka4 Re3 Ka5 Qb3 Ka6 Kd6 Ka5 Kd5 Ka6 Qb4 Ka7 Re6 Ka8 Re1 Ka7 f4 Ka6 f5 Ka7 f6 Ka6 f7 Ka7 f8=Q Ka6 Qf6+ Ka7 g4 Ka8 g5 Ka7 g6 Ka8 g7 Ka7 Qg6 Ka8 h8=Q+ Ka7 Qh8b2 Ka8 Qf4 Ka7 Qc3 Kb7 Qb6+ Kxb6 Qg7 Kb5 Qa4+ Kxa4 Qc3 Kb5 Qf6 Kb4 Ra1 Kb3 Kc5 Kc2 Kc4 Kd2 Qf3 Kc2 Rd1 Kb2 Qf2+ Ka3 Kc3 Ka4 Qf5 Ka3 Qf1 Ka2 Qf2+ Ka3 Kc4 Ka4 Qb2 Ka5 Qa2+ Kb6 Qa8 Kc7 Kc3 Kb6 Kb4 Kc7 Kc4 Kb6 Qc8 Ka5 Qf5+ Kb6 Qb5+ Kc7 Qa6 Kb8 Kd5 Kc7 Ke4 Kb8 Ke5 Kc7 Qc6+ Kxc6 Rd3 Kb5 Kd5 Kb4 Re3 Ka4 Kc5 Ka5 Re1 Ka4 Re3 Ka5

    int human = -2;
    fmt::print("Welcome to Graham's C++ chess.\nWhich color will you play?\n0: white   1: black   2: sim game   -1: free play\n");
    while (human < -1 || human > 2)
        std::cin >> human;

    bool playing = true;
    std::string last_move = "ERROR";
    U64 nodes = 0;
    Timer game_timer;
    SearchLogger sim_log("sim_log", 0);

    // main game loop
    while (playing)
    {
        MoveGenerator mgen(ch);
        std::vector<move> move_list = engine.order_moves_by_piece(ch, mgen.gen_moves());
        if (move_list.size() == 0 || ch.repetitions() > 2) playing = false;

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
        {
            for (move mv : move_list)
                fmt::print("{} ", MoveGenerator::move_san(ch, mv));
            fmt::print("\n{} ", ch.black_to_move ? "Black to move: " : "White to move: ");
        }

        // check for some errors
        if (ch.zhash != ch.hash())
        {
            fmt::print("hash fail!");
            break;
        }
        if (!ch.repetitions())
        {
            fmt::print("rep fail!");
            break;
        }

        // get input
        game_timer.reset();
        std::string str;
        if (!ch.black_to_move && human == 0 || ch.black_to_move && human == 1 || human == -1)
        std::cin >> str;

        // if no input, get ai move
        if (!ch.black_to_move && human == 1 || ch.black_to_move && human == 0 || human == 2)
        {
            // move engine_move = players[ch.black_to_move].iterative_search(ch, SIM_DEPTH, nodes, false);
            move engine_move = engine.iterative_search(ch, SIM_DEPTH, nodes, false);
            last_move = MoveGenerator::move_san(ch, engine_move);
            ch.make_move(engine_move);
            if (human == 2)
                sim_log.write(last_move + " ");
        }
        // otherwise, handle input
        else if (str.substr(0, 2) == "um" && ch.history.size() > 0)
        {
            int undos = 0;
            while (undos < 1 || undos > (int) ch.history.size())
                std::cin >> undos;
            ch.unmake_move(undos);
        }
        else if(str == "help" || str == "?")
        {
            for (std::string tip : HELP_STRINGS)
                fmt::print("{}", tip);
        }
        else if (str == "aim")
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
        else if (str == "perft")
        {
            int depth = 0;
            while (depth < 1)
                std::cin >> depth;
            perft_root(ch, depth, !ch.history.size(), 1);
        }
        else if (str == "test")
            fmt::print("test hash XOR ep_file: {:0>16x}", ch.hash() ^ TTable::ep_file[Compass::file_xindex(ch.ep_square)]);
        else if (str == "end")
            playing = false;
        else for (move mv : move_list)
        {
            if (str != MoveGenerator::move_san(ch, mv))
                continue;
            last_move = MoveGenerator::move_san(ch, mv);
            ch.make_move(mv, true);
            break;
        }
    }

    MoveGenerator mate_gen(ch);
    fmt::print("\n");
    if (ch.repetitions() == 3)
        fmt::print("Draw by repitition!\n");
    else if (mate_gen.in_check)
        fmt::print("{} wins!\neval: {}", ch.black_to_move ? "White" : "Black", engine.eval(ch, 0));
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
    log_depth = min(log_depth, depth);
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
    std::vector<move> moves = perft_gen.gen_moves();
    for (int mvidx = 0; mvidx < (int) moves.size(); mvidx++)
    {
        move mv = moves.at(mvidx);
        std::cout << fmt::format("{}/{}:\t{} ", mvidx + 1, moves.size(), Move::to_string(mv));
        if (depth > perft_log.depth)
        {
            perft_log.buffer = fmt::format("{}/{}:\t{} ", mvidx + 1, moves.size(), Move::to_string(mv));
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
        perft_log.write(fmt::format("{}: finished in {}s at {} nodes/s\n{} moves found.\n",
            SearchLogger::time_to_string(), perft_timer.elapsed(), nodes_per_second, nodes));
    std::cout << fmt::format("{}: finished in {}s at {} nodes/s\n{} moves found\n",
            SearchLogger::time_to_string(), perft_timer.elapsed(), nodes_per_second, nodes);

    // initial position tested, verify results
    if (initial_pos)
    {
        if (depth > perft_log.depth)
        {
            perft_log.write(fmt::format("{} moves expected. ", PERFT_RESULTS[depth]));
            if (nodes != std::stoll(PERFT_RESULTS[depth]))
                perft_log.write("Uh oh!\n");
            else
                perft_log.write("Nice!\n");
        }
        std::cout << PERFT_RESULTS[depth] << " moves expected. ";
        if (nodes != std::stoll(PERFT_RESULTS[depth]))
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
    std::vector<move> moves = perft_gen.gen_moves();
    for (move mv : moves)
    {
        // std::cout << mv << std::endl;
        if (depth > perft_log.depth)
            perft_log.buffer += fmt::format(" {}", MoveGenerator::move_san(ch, mv));
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
