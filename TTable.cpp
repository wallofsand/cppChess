#include "TTable.h"

MyRNG TTable::rng;
std::uniform_int_distribution<U64> TTable::U64_dist;
U64 TTable::black_to_move, TTable::hits, TTable::clashes, TTable::writes;
U64 TTable::sq_color_type_64x2x6[64][2][6];
U64 TTable::castle_rights[2][2];
U64 TTable::ep_file[8];
int TTable::bin[DEFAULT_SIZE];
Entry TTable::table[TTable::DEFAULT_SIZE];

TTable::TTable()
{
    TTable::hits = 0;
    TTable::clashes = 0;
    TTable::writes = 0;
    // e.g. keep one global instance (per thread)
    TTable::rng.seed(seed_val);
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

void TTable::add_item(U64 key, int depth, int flag, float score)
{
    Entry prev_entry = read(key);
    if (depth < 3 || prev_entry.flag && prev_entry.key == key && prev_entry.depth >= depth)
        return;
    if (prev_entry.flag && prev_entry.key != key)
    {
        clashes++;
        bin[abs(key % DEFAULT_SIZE)]++;
    }
    table[abs(key % DEFAULT_SIZE)] = Entry(key, depth, flag, score);
    writes++;
}

Entry TTable::read(U64 key)
{
    return table[abs(key % DEFAULT_SIZE)];
}

void TTable::rand_test(int n)
{
    for (int i = 0; i < n; i++)
    {
        U64 val = U64_dist(rng);
        bin[abs(val % DEFAULT_SIZE)]++;
    }
    float avg;
    float dev;
    for (int idx = 0; idx < DEFAULT_SIZE; idx++)
    {
        // fmt::print("{}: {}\t|", idx, bin[idx]);
        avg = (avg + bin[idx]) / 2.0f;
        dev = (dev + abs(avg - bin[idx])) / 2.0f;
    }
    fmt::print("avg: {}\ndev: {}\n", avg, dev);
    fmt::print("{}\n", U64_dist(rng));
}
