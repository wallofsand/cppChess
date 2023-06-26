#include "TTable.h"

MyRNG TTable::rng;
std::uniform_int_distribution<U64> TTable::U64_dist;
U64 TTable::is_black_turn, TTable::hits, TTable::collisions, TTable::writes, TTable::clears;
U64 TTable::sq_color_type_64x2x6[64][2][6];
// [white, black][king, queen]
U64 TTable::castle_rights_wb_kq[2][2];
U64 TTable::ep_file[8];
Entry TTable::table[TTable::DEFAULT_SIZE];

TTable::TTable() {
    hits = 0;
    collisions = 0;
    writes = 0;
    clears = 0;
    // e.g. keep one global instance (per thread)
    rng.seed(SEED_VAL);
    is_black_turn = U64_dist(rng);
    castle_rights_wb_kq[ch_cst::WHITE_INDEX][0] = U64_dist(rng);
    castle_rights_wb_kq[ch_cst::WHITE_INDEX][1] = U64_dist(rng);
    castle_rights_wb_kq[ch_cst::BLACK_INDEX][0] = U64_dist(rng);
    castle_rights_wb_kq[ch_cst::BLACK_INDEX][1] = U64_dist(rng);
    for (int file = 0; file < 8; file++)
        ep_file[file] = U64_dist(rng);
    for (int sq = 0; sq < 64; sq++)
        for (int type = 0; type < 6; type++)
            for (int color = 0; color < 2; color++)
                sq_color_type_64x2x6[sq][color][type] = U64_dist(rng);
}

/*
 * Method to empty the transposition table.
 */
void TTable::clear() {
    std::cout << "Table cleared. " << ++clears << std::endl;
    writes = 0;
    hits = 0;
    for (int idx = 0; idx < DEFAULT_SIZE; idx++) {
        table[idx].key = 0;
        table[idx].depth = -100;
        table[idx].flag = 0;
        table[idx].score = 0.0f;
        table[idx].best = 0;
    }
}

/*
 * Method to calculate how full the transposition table is
 * @return the percent of Entries in the t-table that have been written too
 */
float TTable::fill_ratio() {
    float num_elements = 0;
    for (Entry e : table)
        num_elements += e.flag > 0;
    return num_elements / DEFAULT_SIZE;
}

int TTable::hash_index(U64 key) {
    return std::abs((int) (key % DEFAULT_SIZE));
}

void TTable::add_item(U64 key, int8_t depth, uint8_t flag, float score, move mv) {
    int index = hash_index(key);
    if (writes / DEFAULT_SIZE > 0.7)
        clear();
    // if hash_index(key) is full, find the next empty index
    while (read(index).flag && read(index).key != key)
        index++;
    // if the position is already searched to a greater depth, do not write
    if (read(index).depth > depth)
        return;
    // record a collision
    if (index != hash_index(key))
        collisions++;
    table[index] = Entry(key, depth, flag, score, mv);
    writes++;
}

Entry& TTable::probe(U64 key) {
    int index = hash_index(key);
    while (read(index).flag && read(index).key != key)
        index++;
    return read(index);
}

Entry& TTable::read(U64 key) {
    return table[hash_index(key)];
}
