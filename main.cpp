#include "Chess.h"
#include "Move.h"
#include "MoveGenerator.h"
#include <iostream>

using namespace ch_cst;

// U64 perft_root(Chess& chess, int depth, bool initial_position, int log_depth = 0);
// U64 perft(MoveGenerator& perft_gen, Chess& chess, int depth, SearchLogger& perft_log);
void print_board(Chess ch, bool fmt = true);
void print_U64(U64 bb, bool fmt = true);

int main()
{
    Compass::init_compass();
    Chess chess;
    MoveGenerator perft_gen(chess);

    MoveGenerator::perft_root(perft_gen, chess, 1, true, 1);

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
