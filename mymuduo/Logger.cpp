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

void Logger::log(std::string msg) {
    std::string level;
    switch(logLevel_) {
        case INFO:
            level = "[INFO] ";
            break;
        case ERROR:
            level = "[ERROR] ";
            break;
        case FATAL:
            level = "[FATAL] ";
            break;
        case DEBUG:
            level = "[DEBUG] ";
            break;
        default:
            break;
    }

    std::cout << level << Timestamp::now().toString() << ": " << msg << std::endl;
}

