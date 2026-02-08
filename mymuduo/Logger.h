#pragma once

#include "noncopyable.h"

#include <cstdio>
#include <string>
#include <cstring>

#define __FILENAME__ (strrchr("/" __FILE__, '/') + 1)

#define LOG_INFO(logmsgFormat, ...) \
    do { \
        Logger &logger = Logger::instance(); \
        logger.setLogLevel(Logger::INFO); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
        logger.log(buf); \
    } while(0)

#define LOG_ERROR(logmsgFormat, ...) \
    do { \
        Logger &logger = Logger::instance(); \
        logger.setLogLevel(Logger::ERROR); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
        logger.log(buf); \
    } while(0)

#define LOG_FATAL(logmsgFormat, ...) \
    do { \
        Logger &logger = Logger::instance(); \
        logger.setLogLevel(Logger::FATAL); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
        logger.log(buf); \
        exit(-1); \
    } while(0)

#ifdef MUDEBUG
    #define LOG_DEBUG(logmsgFormat, ...) \
        do { \
            Logger &logger = Logger::instance(); \
            logger.setLogLevel(Logger::DEBUG); \
            char buf[1024] = {0}; \
            snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__); \
            logger.log(buf); \
        } while(0)
#else
    #define LOG_DEBUG(logmsgFormat, ...)
#endif    


class Logger: noncopyable {
private:
    Logger() {}

public:
    enum LogLevel {
        INFO, ERROR, FATAL, DEBUG
    };
    static Logger& instance();
    void setLogLevel(int level);
    void log(std::string msg);

private:
    int logLevel_;
};

