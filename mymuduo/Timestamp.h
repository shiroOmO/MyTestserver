#pragma once

#include <cstdint>
#include <string>


class Timestamp {
public:
    Timestamp();
    explicit Timestamp(int64_t msSince1900);

    static Timestamp now();
    std::string toString() const;

private:
    int64_t msSince1900_;
};

