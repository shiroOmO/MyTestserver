#pragma once

#include "Callbacks.h"
#include <cstdint>


class Timer;

class TimerId {
public:
    TimerId();
    TimerId(TimerPtr timerPtr, int64_t seq);

    friend class TimerQueue;

private:
    TimerPtr timerPtr_;
    int64_t sequence_;
};


