#pragma once

#include "Callbacks.h"
#include "Timestamp.h"
#include "noncopyable.h"

#include <atomic>
#include <cstdint>


class Timer: noncopyable {
public:
    Timer(TimerCallback cb, Timestamp when, double interval);

    void run() const;
    void restart(Timestamp now); 

    Timestamp expiration() const;
    bool repeat() const;
    int64_t sequence() const;

    static int64_t numCreated();

private:
    const TimerCallback callback_;
    Timestamp expiration_;
    const double interval_;
    const bool repeat_;
    const int64_t sequence_;

    static std::atomic_uint64_t s_numCreated_;
};



