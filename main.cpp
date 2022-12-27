#include "Bitboard.h"
#include "Compass.h"
#include <iostream>

int main() {
    Compass compass;
    for (int sq = 0; sq < 64; sq++)
    {
        std::cout << sq << ":" << std::endl;
        print_binary_string(compass.knight_moves[sq]);
    }
    return 0;
}
