#include "Chess.h"
#include "MoveGenerator.h"
#include <iostream>

char piece_char(int piece);
void print_U64(U64 bb, bool fmt = true);
void print_board(Chess ch);
U64 gen_shift(U64 x, int s);

int main()
{
    Compass::init_compass();
    Chess ch;
    MoveGenerator mgen;

    mgen.perft_root(3);
    // print_board(ch);

    for (Move mv : mgen.gen_moves())
    {
        // ch.make_move(mv);
        // mgen.set_chess(&ch);
        // std::cout << mv << std::endl;
        // mgen.perft_root(2);
        // ch.unmake_move(1);
    }

    // while (!ch.game_over)
    // {
    //     mgen.set_chess(&ch);
    //     std::vector<Move> move_list = mgen.gen_moves();
    //     print_board(ch);
    //     bool move = false;
    //     Move next(0,0);
    //     while (!move)
    //     {
    //         int counter = 0;
    //         for (Move m : mgen.gen_moves())
    //         {
    //             std::cout << "(" << Compass::string_from_square(m.start) << ", " << Compass::string_from_square(m.end) << ") ";
    //             if (counter % 5 == 4)
    //                 std::cout << std::endl;
    //             counter++;
    //         }
    //         std::cout << std::endl << "Start square: ";
    //         std::string startstr, endstr;
    //         std::cin >> startstr;
    //         std::cout << " End square: ";
    //         std::cin >> endstr;
    //         std::cout << std::endl;
    //         int start = Compass::square_from_string(startstr);
    //         int end = Compass::square_from_string(endstr);
    //         for (Move m : move_list)
    //         {
    //             if (m.start == start && m.end == end && (m.promote == 0 || m.promote == ch_cst::QUEEN))
    //             {
    //                 move = true;
    //                 next = Move(start, end, m.promote);
    //             }
    //         }
    //     }
    //     ch.make_move(next);
    // }

    return 0;
}

char piece_char(int piece)
{
    switch (piece)
    {
    case 1:
        return 'P';
    case 9:
        return 'p';
    case 2:
        return 'N';
    case 10:
        return 'n';
    case 3:
        return 'B';
    case 11:
        return 'b';
    case 4:
        return 'R';
    case 12:
        return 'r';
    case 5:
        return 'Q';
    case 13:
        return 'q';
    case 6:
        return 'K';
    case 14:
        return 'k';
    default:
        return '.';
    }
}

void print_board(Chess ch)
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
                    board += piece_char(piece | (color << 3));
                }
            }
        }
        if (board.length() > sq)
            continue;
        board += '.';
    }
    Bitboard::print_binary_string(board);
}

void print_U64(U64 bb, bool fmt)
{
    Bitboard::print_binary_string(Bitboard::build_binary_string(bb), fmt);
}
