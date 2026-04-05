#include "EventLoop.h"
#include "Logger.h"
#include "Timer.h"
#include "TimerQueue.h"

#include <ctime>
#include <sys/timerfd.h>
#include <unistd.h>


int createTimerfd() {
    int timerfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if(timerfd < 0)
        LOG_FATAL("Failed in timerfd_create, exit.");

    return timerfd;
}

struct timespec howMuchTimeFromNow(Timestamp when) {
    int64_t microseconds = when.msSince1900() - Timestamp::now().msSince1900();
    if(microseconds < 100)
        microseconds = 100;

    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(microseconds / (1000 * 1000));
    ts.tv_nsec = static_cast<long>(microseconds % (1000 * 1000) * 1000);

    return ts;
}

void resetTimerfd(int timerfd, Timestamp expiration) {
    struct itimerspec newValue;
    struct itimerspec oldValue;
    memset(&newValue, 0, sizeof newValue);
    memset(&oldValue, 0, sizeof oldValue);
    newValue.it_value = howMuchTimeFromNow(expiration);
    int ret = timerfd_settime(timerfd, 0, &newValue, &oldValue);
    if(ret)
        LOG_FATAL("Fatal in timerfd_settime.");
}

void readTimerfd(int timerfd) {
    uint64_t howmany;
    ssize_t n = read(timerfd, &howmany, sizeof howmany);
    if(n != sizeof howmany)
        LOG_ERROR("TimerQueue::handleRead() reads %ld bytes instead of 8.", n);
}

TimerQueue::TimerQueue(EventLoop *loop): loop_(loop), timerfd_(createTimerfd()),
    timerfdChannel_(loop, timerfd_), callingExpiredTimers_(false) {

    timerfdChannel_.setReadCallback(std::bind(&TimerQueue::handleRead, this));
    timerfdChannel_.enableReading();
}

TimerQueue::~TimerQueue() {
    timerfdChannel_.disableAll();
    timerfdChannel_.remove();
    close(timerfd_);
}

TimerId TimerQueue::addTimer(TimerCallback cb, Timestamp when, double interval) {
    TimerPtr timerPtr(new Timer(std::move(cb), when, interval));
    loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, timerPtr));
    return TimerId(timerPtr, timerPtr->sequence());
}

void TimerQueue::cancel(TimerId timerId) {
    loop_->runInLoop(std::bind(&TimerQueue::cancelInLoop, this, timerId));
}

void TimerQueue::addTimerInLoop(TimerPtr timerPtr) {
    bool earliestChanged = insert(timerPtr);
    if(earliestChanged)
        resetTimerfd(timerfd_, timerPtr->expiration());
}

void TimerQueue::cancelInLoop(TimerId timerId) {
    ActiveTimer timer(timerId.timerPtr_, timerId.sequence_);
    ActiveTimerSet::iterator it = activeTimers_.find(timer);
    if(it != activeTimers_.end()) {
        timers_.erase(Entry(it->first->expiration(), it->first));
        activeTimers_.erase(it);
    }
    else if(callingExpiredTimers_)
        cancelingTimers_.insert(timer);
}

void TimerQueue::handleRead() {
    readTimerfd(timerfd_);
    Timestamp now(Timestamp::now());

    std::vector<Entry> expired = getExpired(now);
    callingExpiredTimers_ = true;
    cancelingTimers_.clear();

    for(const Entry &it: expired)
        it.second->run();
    callingExpiredTimers_ = false;
    reset(expired, now);
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now) {
    std::vector<Entry> expired;
    Entry sentry(now, nullptr); 
    TimerList::iterator end = timers_.upper_bound(sentry);
    std::copy(timers_.begin(), end, back_inserter(expired));
    LOG_DEBUG("expired.size() == %ld, timers_.size() == %ld", expired.size(), timers_.size());
    timers_.erase(timers_.begin(), end);

    for(const Entry &it: expired) {
        ActiveTimer timer(it.second, it.second->sequence());
        activeTimers_.erase(timer);
    }

    return expired;
}

bool TimerQueue::insert(TimerPtr timerPtr) {
    bool earliestChanged = false;
    Timestamp when = timerPtr->expiration();
    TimerList::iterator it = timers_.begin();
    if(it == timers_.end() || when < it->first)
        earliestChanged = true;

    timers_.insert(Entry(when, timerPtr));
    activeTimers_.insert(ActiveTimer(timerPtr, timerPtr->sequence()));
    return earliestChanged;
}

void TimerQueue::reset(const std::vector<Entry> &expired, Timestamp now) {
    Timestamp nextExpire;
    for(const Entry &it: expired) {
        ActiveTimer timer(it.second, it.second->sequence());
        if(it.second->repeat() && cancelingTimers_.find(timer) == cancelingTimers_.end()) {
            it.second->restart(now);
            insert(it.second);
        }
    }
    if(!timers_.empty())
        nextExpire = timers_.begin()->second->expiration();

    if (nextExpire.valid())
        resetTimerfd(timerfd_, nextExpire);
}


