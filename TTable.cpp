#include "TTable.h"

MyRNG TTable::rng;
std::uniform_int_distribution<U64> TTable::U64_dist;
U64 TTable::is_black_turn, TTable::hits, TTable::collisions, TTable::writes;
U64 TTable::sq_color_type_64x2x6[64][2][6];
// [white, black][king, queen]
U64 TTable::castle_rights_wb_kq[2][2];
U64 TTable::ep_file[8];
Entry TTable::table[TTable::DEFAULT_SIZE];

TTable::TTable()
{
    TTable::hits = 0;
    TTable::collisions = 0;
    TTable::writes = 0;
    // e.g. keep one global instance (per thread)
    TTable::rng.seed(SEED_VAL);
    TTable::is_black_turn = U64_dist(rng);
    TTable::castle_rights_wb_kq[ch_cst::WHITE_INDEX][0] = U64_dist(rng);
    TTable::castle_rights_wb_kq[ch_cst::WHITE_INDEX][1] = U64_dist(rng);
    TTable::castle_rights_wb_kq[ch_cst::BLACK_INDEX][0] = U64_dist(rng);
    TTable::castle_rights_wb_kq[ch_cst::BLACK_INDEX][1] = U64_dist(rng);
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
float TTable::fill_ratio()
{
    float num_elements = 0;
    for (Entry e : table)
        num_elements += (e.flag != 1);
    return num_elements / DEFAULT_SIZE;
}

int TTable::hash_index(U64 key)
{
    return std::abs((int) key % DEFAULT_SIZE);
}

void TTable::add_item(U64 key, int8_t depth, uint8_t flag, float score, move mv)
{
    int index = hash_index(key);
    // if hash_index(key) is full, find the next empty index
    while (read(index).flag && read(index).key != key)
        index++;
    // if the position is already searched to a greater depth or if
    // the position is already searched to a more exact result, do not write
    if (read(index).depth > depth || read(index).depth == depth && flag != Entry::FLAG_EXACT)
        return;
    // record a collision
    if (index != hash_index(key))
        collisions++;
    table[index] = Entry(key, depth, flag, score, mv);
    writes++;
}

Entry TTable::probe(U64 key)
{
    int index = hash_index(key);
    while (read(index).flag && read(index).key != key)
        index++;
    return read(index);
}

Entry TTable::read(U64 key)
{
    return table[hash_index(key)];
}
