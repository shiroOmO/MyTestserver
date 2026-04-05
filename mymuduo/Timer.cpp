#include "Timer.h"


std::atomic_uint64_t Timer::s_numCreated_(0);

Timer::Timer(TimerCallback cb, Timestamp when, double interval):
    callback_(std::move(cb)), expiration_(when), interval_(interval),
    repeat_(interval > 0.0), sequence_(s_numCreated_.fetch_add(1)) {

    }

void Timer::run() const {
    callback_();
}

void Timer::restart(Timestamp now) {
    if(repeat_)
        expiration_ = addTime(now, interval_);
    else
        expiration_ = Timestamp();
}

Timestamp Timer::expiration() const {
    return expiration_;
}

bool Timer::repeat() const {
    return repeat_;
}

int64_t Timer::sequence() const {
    return sequence_;
}

int64_t Timer::numCreated() {
    return s_numCreated_.load();
}


