#pragma once

#include "Callbacks.h"
#include "Channel.h"
#include "noncopyable.h"
#include "TimerId.h"

#include <cstdint>
#include <vector>
#include <set>


class EventLoop;
class Timer;
class Timestamp;

class TimerQueue: noncopyable {
public:
    typedef std::pair<Timestamp, TimerPtr> Entry;
    typedef std::pair<TimerPtr, int64_t> ActiveTimer;
    typedef std::set<Entry> TimerList;
    typedef std::set<ActiveTimer> ActiveTimerSet;

    explicit TimerQueue(EventLoop *loop);
    ~TimerQueue();

    TimerId addTimer(TimerCallback cb, Timestamp when, double interval);
    void cancel(TimerId timerId);

private:
    void addTimerInLoop(TimerPtr timerPtr);
    void cancelInLoop(TimerId timerId);
    void handleRead();

    std::vector<Entry> getExpired(Timestamp now);
    bool insert(TimerPtr timerPtr);
    void reset(const std::vector<Entry> &expired, Timestamp now);

private:
    EventLoop *loop_;
    const int timerfd_;
    Channel timerfdChannel_;
    bool callingExpiredTimers_;
    
    TimerList timers_;
    ActiveTimerSet activeTimers_;
    ActiveTimerSet cancelingTimers_;
};





