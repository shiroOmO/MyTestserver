#include "Timestamp.h"

#include <cstdint>
#include <string>
#include <time.h>
#include <cstdio>


Timestamp::Timestamp(): msSince1900_(0) {}

Timestamp::Timestamp(int64_t msSince1900): msSince1900_(msSince1900) {}

Timestamp Timestamp::now() {
    return Timestamp(time(nullptr));
}

std::string Timestamp::toString() const {
    char buf[128];
    tm *tm_time = localtime(&msSince1900_);
    snprintf(buf, 128, "%4d/%02d/%02d %02d:%02d:%02d",
            tm_time->tm_year + 1900,
            tm_time->tm_mon + 1,
            tm_time->tm_mday,
            tm_time->tm_hour,
            tm_time->tm_min,
            tm_time->tm_sec);

    return buf;
}



