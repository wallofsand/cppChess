#include "SearchLogger.h"

using namespace ch_cst;

SearchLogger::SearchLogger(std::string str, int d)
{
    name = str;
    depth = d;
    buffer = "";
}

/*
 * Write a string to the log file
 */
const void SearchLogger::write(std::string text)
{
    std::string file_path =
        "C:/Users/graha/Documents/GitHub/cppChess/logs/"
        + name + "_" + SearchLogger::date_to_string() + ".txt";
    fmt::ostream out = fmt::output_file(file_path,
        fmt::file::WRONLY | fmt::file::CREATE | fmt::file::APPEND);
    out.print(text);
}

/*
 * @return the current date as a string 20XX-12-05
 */
std::string SearchLogger::date_to_string()
{
    return fmt::format("{:%Y-%m-%d}", fmt::localtime(std::time(nullptr)));
}

/*
 * @return the current time in strftime-like format: 03:15:30
 */
std::string SearchLogger::time_to_string()
{
    return fmt::format("{:%H:%M:%S}", fmt::localtime(std::time(nullptr)));
}
