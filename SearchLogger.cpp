#include "SearchLogger.h"

using namespace ch_cst;

/*
 * Initialize the log file
 */
void SearchLogger::init()
{
    std::string file_path = "C:/Users/graha/Documents/GitHub/cppChess/log/"
                          + SearchLogger::date_to_string();
}

/*
 * @return the current date as a string
 */
const std::string SearchLogger::date_to_string()
{
    std::time_t t = std::time(nullptr);
    return fmt::format("{:%Y/%m/%d}", fmt::localtime(t));
}

// inline string getCurrentDateTime( string s ){
//     time_t now = time(0);
//     struct tm  tstruct;
//     char  buf[80];
//     tstruct = *localtime(&now);
//     if(s=="now")
//         strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);
//     else if(s=="date")
//         strftime(buf, sizeof(buf), "%Y-%m-%d", &tstruct);
//     return string(buf);
// };
// inline void Logger( string logMsg ){

//     string filePath = "/somedir/log_"+getCurrentDateTime("date")+".txt";
//     string now = getCurrentDateTime("now");
//     ofstream ofs(filePath.c_str(), std::ios_base::out | std::ios_base::app );
//     ofs << now << '\t' << logMsg << '\n';
//     ofs.close();
// }