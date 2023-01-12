#include "TTable.h"

MyRNG TTable::rng;
long TTable::black_to_move, TTable::hits, TTable::clashes, TTable::writes;
long TTable::sq_color_type_64x2x6[64][2][6];
long TTable::castle_rights[2][2];
long TTable::ep_file[8];
Position TTable::table[TTable::DEFAULT_SIZE];

TTable::TTable()
{
    TTable::hits = 0;
    TTable::clashes = 0;
    TTable::writes = 0;
    // e.g. keep one global instance (per thread)
    TTable::rng.seed(seed_val);
    std::uniform_int_distribution<U64> U64_dist;
    TTable::black_to_move = U64_dist(rng);
    TTable::castle_rights[ch_cst::WHITE_INDEX][0] = U64_dist(rng);
    TTable::castle_rights[ch_cst::WHITE_INDEX][1] = U64_dist(rng);
    TTable::castle_rights[ch_cst::BLACK_INDEX][0] = U64_dist(rng);
    TTable::castle_rights[ch_cst::BLACK_INDEX][1] = U64_dist(rng);
    for (int file = 0; file < 8; file++)
        TTable::ep_file[file] = U64_dist(rng);
    for (int sq = 0; sq < 64; sq++)
        for (int type = 0; type < 6; type++)
            for (int color = 0; color < 2; color++)
                TTable::sq_color_type_64x2x6[sq][color][type] = U64_dist(rng);
}

void TTable::entry(long key, int depth, int flag, float score)
{
    if (read(key).flag && read(key).key != key)
        clashes++;
    else if (read(key).key == key && read(key).depth >= depth)
        return;
    table[abs(key % DEFAULT_SIZE)] = Position(key, depth, flag, score);
    writes++;
}

Position TTable::read(long key)
{
    return table[abs(key % DEFAULT_SIZE)];
}
