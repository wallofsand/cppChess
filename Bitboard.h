#ifndef BITBOARD_H
#define BITBOARD_H

#include <string>
#include <iostream>

#define ULL unsigned long long

bool contains_square(int sq);
std::string build_binary_string(ULL bb);
void print_binary_string(std::string bbstr);
int leading_zeros(ULL num);

#endif
