// Logging class for move searches and perft verification
// GB 2023/01/03
#ifndef SEARCHLOGGER_H
#define SEARCHLOGGER_H

#include <string>
#include <iostream>
#include <ostream>
#include <windows.h>
#include "Move.h"
#include "Compass.h"
#include "Bitboard.h"
#include "fmt/include/fmt/format.h"
#include "fmt/include/fmt/os.h"
#include "fmt/include/fmt/chrono.h"

class SearchLogger {
public:
    inline SearchLogger() : out(fmt::output_file(file_path(),
            fmt::file::WRONLY | fmt::file::CREATE | fmt::file::APPEND)) {}
    inline SearchLogger(std::string str, int d) : name(str), depth(d), out(fmt::output_file(file_path(),
            fmt::file::WRONLY | fmt::file::CREATE | fmt::file::APPEND)) {}
    inline SearchLogger(std::string str, int d, int params) : name(str), depth(d), out(fmt::output_file(file_path(), params)) {}
    inline ~SearchLogger() { out.close(); }
    const std::string file_path() const;
    void write(std::string text);
    void log_position(std::string mv_text);
    static std::string date_to_string();
    static std::string time_to_string();
    int depth;
    std::string buffer = "";
private:
    std::string name;
    fmt::ostream out;
};

#endif
