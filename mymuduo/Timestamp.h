#pragma once

#include <cstdint>
#include <string>


class Timestamp {
public:
    Timestamp();
    explicit Timestamp(int64_t msSince1900);

    int64_t msSince1900() const;
    static Timestamp now();
    std::string toString() const;

    bool operator <(const Timestamp &rv) const;
    bool valid() const;

private:
    int64_t msSince1900_;
};

inline Timestamp addTime(Timestamp timestamp, double seconds) {
    int64_t delta = static_cast<int64_t>(seconds * 1000 * 1000);
    return Timestamp(timestamp.msSince1900() + delta);
}


