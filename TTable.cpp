#include "TTable.h"

MyRNG TTable::rng;
std::uniform_int_distribution<U64> TTable::U64_dist;
U64 TTable::black_to_move, TTable::hits, TTable::collisions, TTable::writes;
U64 TTable::sq_color_type_64x2x6[64][2][6];
U64 TTable::castle_rights[2][2];
U64 TTable::ep_file[8];
// int TTable::bin[DEFAULT_SIZE];
Entry TTable::table[TTable::DEFAULT_SIZE];

TTable::TTable()
{
    TTable::hits = 0;
    TTable::collisions = 0;
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

/*
 * Method to calculate how full the transposition table is
 * @return the percent of Entries in the t-table that have been written too
 */
const float TTable::fill_ratio()
{
    float num_elements = 0;
    for (Entry e : table)
    {
        if (e.flag) num_elements++;
    }
    return num_elements / DEFAULT_SIZE;
}

const int TTable::hash_index(U64 key)
{
    return std::abs((int) key % DEFAULT_SIZE);
}

void TTable::add_item(U64 key, int8_t depth, uint8_t flag, float score, move mv)
{
    int index = hash_index(key);
    // if hash_index(key) is full, find the next empty index
    while (read(index).flag != 0 && read(index).key != key)
        index++;
    // record a collision
    if (index != hash_index(key))
        collisions++;
    // if the position is already searched to a greater depth, do not write
    if (depth < -6 || read(index).flag && read(index).depth >= depth)
        return;
    table[index] = Entry(key, depth, flag, score, mv);
    writes++;
}

const Entry TTable::probe(U64 key)
{
    int index = hash_index(key);
    while (read(index).flag && read(index).key != key)
        index++;
    return read(index);
}

const Entry TTable::read(U64 key)
{
    return table[hash_index(key)];
}

// void TTable::rand_test(int n)
// {
//     for (int i = 0; i < n; i++)
//         bin[hash_index(U64_dist(rng))]++;
//     float avg = 0;
//     float dev = 0;
//     for (int idx = 0; idx < DEFAULT_SIZE; idx++)
//     {
//         // fmt::print("{}: {}\t|", idx, bin[idx]);
//         avg = (avg + bin[idx]) / 2.0f;
//         dev = (dev + abs(avg - bin[idx])) / 2.0f;
//     }
//     fmt::print("avg: {}\ndev: {}\n", avg, dev);
//     fmt::print("{}\n", U64_dist(rng));
// }
