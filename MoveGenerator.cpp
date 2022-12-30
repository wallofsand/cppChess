#include "MoveGenerator.h"

MoveGenerator::MoveGenerator(Chess* ch)
{
    chess = ch;
}

std::vector<Move> MoveGenerator::gen_moves()
{
    std::vector<Move> moves;
    for (int sq = 0; sq < 64; sq++)
    {
        if (!Bitboard::contains_square(*chess->color_bbs[chess->active_color_index], sq))
            continue;
        if (Bitboard::contains_square(*chess->piece_bbs[constants::PAWN], sq))
        {
            gen_pawn_piece_moves(sq);
        }
        else if (Bitboard::contains_square(*chess->piece_bbs[constants::KNIGHT], sq))
        {
            gen_knight_piece_moves(sq);
        }
        else if (Bitboard::contains_square(*chess->piece_bbs[constants::KING], sq))
        {
            gen_king_piece_moves(sq);
        }
        else if (Bitboard::contains_square(*chess->piece_bbs[constants::BISHOP], sq))
        {
            gen_sliding_piece_moves(sq, constants::BISHOP);
        }
        else if (Bitboard::contains_square(*chess->piece_bbs[constants::ROOK], sq))
        {
            gen_sliding_piece_moves(sq, constants::ROOK);
        }
        else if (Bitboard::contains_square(*chess->piece_bbs[constants::QUEEN], sq))
        {
            gen_sliding_piece_moves(sq, constants::QUEEN);
        }
    }
    return moves;
}

std::vector<Move> MoveGenerator::gen_pawn_piece_moves(int sq)
{
    std::vector<Move> pawn_moves;

    return pawn_moves;
}

std::vector<Move> MoveGenerator::gen_knight_piece_moves(int sq)
{
    std::vector<Move> knight_moves;
    ULL moves = Compass::KNIGHT_ATTACKS[sq] & ~*chess->color_bbs[chess->active_color_index];
    for (int end = 0; end < 64; end++)
    {
        if (!Bitboard::contains_square(moves, end))
            continue;
        knight_moves.push_back(Move(sq, end));
    }
    return knight_moves;
}

std::vector<Move> MoveGenerator::gen_sliding_piece_moves(int sq, int piece)
{
    std::vector<Move> slider_moves;
    
    return slider_moves;
}

std::vector<Move> MoveGenerator::gen_king_piece_moves(int sq)
{
    std::vector<Move> king_moves;

    return king_moves;
}
