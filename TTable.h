#ifndef TTABLE_H
#define TTABLE_H

#include "Move.h"
#include "Compass.h"
#include "fmt/include/fmt/format.h"
#include <random>
#include <iostream>

typedef std::mt19937_64 MyRNG;

struct Entry {
    Entry() : key(0), depth(-100), flag(0), score(0.0f), best(0) {}
    Entry(U64 k, int8_t d, uint8_t f, float score) : key(k), depth(d), flag(f), score(score), best(0) {}
    Entry(U64 k, int8_t d, uint8_t f, float score, move m) : key(k), depth(d), flag(f), score(score), best(m) {}
    U64 key;
    int8_t depth;
    uint8_t flag;
    float score;
    move best;
    inline std::string to_string() const
    { return fmt::format("key: {} depth: {} flag: {} score: {} best: {}",
        key, depth, flag, score, best); };
    // eval is exact value (all moves were searched)
    static const uint8_t FLAG_EXACT = 1;
    // eval is < alpha value
    static const uint8_t FLAG_ALPHA = 2;
    // eval is > beta value
    static const uint8_t FLAG_BETA = 3;
};

class TTable {
public:
    TTable();
    // smaller table cleared every move
    // static const int DEFAULT_SIZE = 5595979;
    static const int DEFAULT_SIZE = 35000011;
    static const U64 SEED_VAL = 15375420585056461361ull;

    // the Mersenne Twister with a popular choice of parameters
    static MyRNG rng;
    static std::uniform_int_distribution<U64> U64_dist;

    // array of random bitstrings for each piece at each square
    static U64 sq_color_type_64x2x6[64][2][6];

    // Kingside: 0, Queenside: 1
    static U64 castle_rights_wb_kq[2][2];

    // file 0 - 7 of ep square
    static U64 ep_file[8];

    static U64 is_black_turn;
    static U64 hits, collisions, writes;

    static void clear();
    static float fill_test();
    static float fill_ratio();
    static int hash_index(U64 key);
    static void add_item(U64 key, int8_t depth, uint8_t flag, float score, move mv = 0);
    static Entry read(U64 key);
    static Entry probe(U64 key);
private:
    static Entry table[DEFAULT_SIZE];
};

#endif
