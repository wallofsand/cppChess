#ifndef TTABLE_H
#define TTABLE_H

#include "Move.h"
#include "Compass.h"
#include "fmt/include/fmt/format.h"
#include <random>
#include <iostream>

typedef std::mt19937_64 MyRNG;

struct Position
{
    Position() : key(0), depth(0), flag(0), score(0) {}
    Position(long k, int d, int f, float score) : key(k), depth(d), flag(f), score(score) {}
    long key;
    int depth;
    int flag;
    float score;
    inline const std::string to_string()
    { return fmt::format("key: {} depth: {} flag: {} score: {}",
        key, depth, flag, score); };
    // eval is exact value
    static const int FLAG_EXACT = 1;
    // eval is > maximum value
    static const int FLAG_ALPHA = 2;
    // eval was > beta cutoff
    static const int FLAG_BETA = 3;
};

class TTable
{
public:
    TTable();

    static const int DEFAULT_SIZE = 64000;
    static const U64 seed_val = 3229840203366374022ll;
    // static const U64 seed_val = 1397063171404053109;
    // the Mersenne Twister with a popular choice of parameters
    static MyRNG rng;
    static long black_to_move;
    // array of random bitstrings for each piece at each square
    // pawns are special case: first "rank" is used for white ep-able pawns
    // while the second "rank" is used for black's ep-able pawns
    static long sq_color_type_64x2x6[64][2][6];
    // short: 0, long: 1
    static long castle_rights[2][2];
    // file 0 - 7 of ep square
    static long ep_file[8];
    static long hits, clashes, writes;
    static void entry(long key, int depth, int flag, float score);
    static void entry(long key, int depth, int flag, float score, Move mv);
    static Position read(long key);
private:
    static Position table[DEFAULT_SIZE];
};

#endif

// package chess3;

// import java.lang.Math;

// public class TranspositionTable {

// 	private int size;
// 	Position[] table;

// 	TranspositionTable(int size) {
// 		this.size = size;
// 		this.table = new Position[size];
// 	}

// 	// create an entry to store in the table
// 	Position makePosition(long zobrist, int depthRemaining, int isExactScore, float score, Move m) {
// //		Zobrist.incrementWrites();
// 		table[(int) Math.abs(zobrist % getSize())] = new Position(zobrist, depthRemaining, isExactScore, score, m);
// 		return table[(int) Math.abs(zobrist % getSize())];
// 	}

// 	// return a value stored in the transposition table
// 	Position getPosition(long hash) {
// 		return table[(int) Math.abs(hash % getSize())];
// 	}

// 	// method to 
// 	void add(Position newPos) {
// 		Position p0 = table[(int) Math.abs(newPos.key % getSize())];
// 		if (p0 == null || p0.key == newPos.key) {
// 			table[(int) Math.abs(newPos.key % getSize())] = newPos;
// //			Zobrist.incrementWrites();
// 		} else {
// 			table[(int) Math.abs(newPos.key % getSize())] = newPos;
// //			Zobrist.incrementClashes();
// //			Zobrist.incrementWrites();
// 		}
// 	}

// 	// Method to get the depth of a position within the ttable
// 	int getDepth(long hash) {
// 		if (!containsPosition(hash))
// 			return -1;
// //		if (table[(int) Math.abs(hash % getSize())].key != hash)
// //			return -1; // return -1 if there is a clash
// 		return table[(int) Math.abs(hash % getSize())].depth;
// 	}

// 	int getSize() {
// 		return size;
// 	}

// 	boolean containsPosition(long hash) { // should we record clashes here?
// 		Position p0 = getPosition(hash);
// 		if (p0 == null) return false;
// 		else if (p0.key == hash) return true;
// //		Zobrist.incrementClashes();
// 		return false;
// //		return getPosition(hash) != null && getPosition(hash).key == hash;
// 	}

// 	float probeTable(long hash, int depth, float alpha, float beta) {
// 		Position p0 = table[(int) Math.abs(hash % getSize())];
// 		if (p0.depth >= depth) {
// 			if (p0.flag == Position.flagEXACT)
// 				return p0.eval;
// 			else if (p0.flag == Position.flagALPHA && p0.eval <= alpha)
// 				return alpha;
// 			else if (p0.flag == Position.flagBETA && p0.eval >= beta)
// 				return beta;
// 		}
// 		return beta;
// 	}

// 	Move getMove(long hash) {
// 		return table[(int) Math.abs(hash % getSize())] != null ? table[(int) Math.abs(hash % getSize())].bestmove
// 				: null;
// 	}

// }
