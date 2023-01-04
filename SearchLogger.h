// 
#ifndef SEARCHLOGGER_H
#define SEARCHLOGGER_H

#include "Move.h"
#include "Compass.h"
#include "Bitboard.h"
#include "fmt/include/fmt/format.h"
#include "fmt/include/fmt/os.h"
#include "fmt/include/fmt/chrono.h"
#include <string>
#include <iostream>
#include <ostream>

class SearchLogger {
public:
    SearchLogger(std::string str, int d);
    std::string buffer;
    const void write(std::string text);
    static std::string date_to_string();
    static std::string time_to_string();
    int depth;
private:
    std::string name;
};

#endif
