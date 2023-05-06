#include "SearchLogger.h"

const std::string SearchLogger::file_path() const {
    CreateDirectory("logs", NULL);
    return "logs/" + name + "_" + SearchLogger::date_to_string() + ".txt";
}

/*
 * Write a string to the log file
 * @param text the string to write
 */
void SearchLogger::write(std::string text) {
    fmt::ostream out = fmt::output_file(file_path(), fmt::file::WRONLY | fmt::file::CREATE | fmt::file::APPEND);
    out.print(text);
}

void SearchLogger::log_position(std::string mv_txt) {
    buffer += ", " + mv_txt;
    write(buffer);
}

/*
 * @return the current date as a string 20XX-12-05
 */
std::string SearchLogger::date_to_string() {
    return fmt::format("{:%Y-%m-%d}", fmt::localtime(std::time(nullptr)));
}

/*
 * @return the current time in strftime-like format: 03:15:30
 */
std::string SearchLogger::time_to_string() {
    return fmt::format("{:%H:%M:%S}", fmt::localtime(std::time(nullptr)));
}
