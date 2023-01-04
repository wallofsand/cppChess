// 
#ifndef SEARCHLOGGER_H
#define SEARCHLOGGER_H

#include "Move.h"
#include "Compass.h"
#include "Bitboard.h"
#include <string>
#include <iostream>
#include <ostream>

class SearchLogger {
public:
    SearchLogger();
    static const std::string date_to_string();
    int flag;
private:
    void init();
    std::string write();
};

#endif
