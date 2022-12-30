#include "Chess.h"
#include "MoveGenerator.h"
#include <iostream>

char piece_char(int piece);
void print_ULL(U64 bb, bool fmt = true);
void print_board(Chess ch);

int main()
{
    Compass::init_compass();
    Chess chess;
    MoveGenerator mgen;
    while (!chess.game_over)
    {
        mgen.set_chess(&chess);
        std::vector<Move> move_list = mgen.gen_moves();
        print_board(chess);
        bool move = false;
        Move* next;
        while (!move)
        {
            int counter = 0;
            for (Move m : mgen.gen_moves())
            {
                if (counter == 4)
                    std::cout << std::endl;
                std::cout << m.start << ", " << m.end << " ";
                counter = counter++ % 5;
            }
            std::cout << std::endl << "Start square: ";
            int start, end;
            std::cin >> start;
            std::cout << " End square: ";
            std::cin >> end;
            for (Move m : move_list)
            {
                if (m.start == start && m.end == end && (m.promote == 0 || m.promote == ch_cst::QUEEN))
                {
                    move = true;
                    next = &m;
                }
            }
        }
        chess.make_move(*next);
    }




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

void print_ULL(U64 bb, bool fmt)
{
    Bitboard::print_binary_string(Bitboard::build_binary_string(bb), fmt);
}
