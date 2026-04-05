#include "Timestamp.h"

#include <string>
#include <sys/time.h>
#include <time.h>
#include <cstdio>


Timestamp::Timestamp(): msSince1900_(0) {}

Timestamp::Timestamp(int64_t msSince1900): msSince1900_(msSince1900) {}

int64_t Timestamp::msSince1900() const {
    return msSince1900_;
}

Timestamp Timestamp::now() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return Timestamp(tv.tv_sec * 1000 * 1000 + tv.tv_usec);
}

std::string Timestamp::toString() const {
    char buf[128];
    time_t sec = static_cast<time_t>(msSince1900_ / (1000 * 1000));
    struct tm tm_time;
    localtime_r(&sec, &tm_time);
    snprintf(buf, 128, "%4d/%02d/%02d %02d:%02d:%02d",
            tm_time.tm_year + 1900,
            tm_time.tm_mon + 1,
            tm_time.tm_mday,
            tm_time.tm_hour,
            tm_time.tm_min,
            tm_time.tm_sec);

    return buf;
}

bool Timestamp::operator <(const Timestamp &rv) const {
    return msSince1900_ < rv.msSince1900();
}

bool Timestamp::valid() const {
    return msSince1900_ > 0;
}

