#include "Logger.h"
#include "Timestamp.h"

#include <iostream>
#include <string>


Logger& Logger::instance() {
    static Logger logger;
    return logger;
}

void Logger::setLogLevel(int level) {
    logLevel_ = level;
}

void Logger::log(const char *file, int line, std::string msg) {
    const char *levelStr[] = {"[INFO]", "[ERROR]", "[FATAL]", "[DEBUG]"};

    std::cout << Timestamp::now().toString() << ' '
        << levelStr[logLevel_] << ' '
        << file << ':' << line << " - "
        << msg << std::endl;
}

