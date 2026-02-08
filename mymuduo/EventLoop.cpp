#include "EventLoop.h"
#include "Channel.h"
#include "CurrentThread.h"
#include "Logger.h"
#include "Poller.h"

#include <cerrno>
#include <cstdint>
#include <memory>
#include <mutex>
#include <sys/eventfd.h>
#include <sys/types.h>
#include <unistd.h>


__thread EventLoop *t_loopInThisThread = nullptr;

const int kPollTimeMs = 10000;

static int createEventfd() {
    int evtfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if(evtfd < 0)
        LOG_FATAL("%s:%s:%d => thread=%d's eventloop eventfd create fail, exit, errno=%d.", __FILENAME__, __FUNCTION__, __LINE__, CurrentThread::tid(), errno);
    return evtfd;
}

EventLoop::EventLoop():
    looping_(false),
    quit_(false),
    callingPendingFunctors_(false),
    threadId_(CurrentThread::tid()),
    poller_(Poller::newDefaultPoller(this)),
    wakeupFd_(createEventfd()),
    wakeupChannel_(new Channel(this, wakeupFd_)) {
    
    LOG_INFO("%s:%s:%d => eventloop=%p create in thread=%d.", __FILENAME__, __FUNCTION__, __LINE__, this, threadId_);
    if(t_loopInThisThread)
        LOG_FATAL("%s:%s:%d => another eventloop=%p exists int this thread=%d, exit.", __FILENAME__, __FUNCTION__, __LINE__, t_loopInThisThread, threadId_);
    else
        t_loopInThisThread = this;

    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop() {
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    close(wakeupFd_);
    t_loopInThisThread = nullptr;
}

void EventLoop::loop() {
    looping_ = true;
    quit_ = false;

    LOG_INFO("%s:%s:%d => thread=%d's eventloop=%p start looping.", __FILENAME__, __FUNCTION__, __LINE__, threadId_, this);

    while(!quit_) {
        activeChannel_.clear();
        pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannel_);
        for(Channel *channel: activeChannel_) {
            channel->handleEvent(pollReturnTime_);
        }
        doPendingFunctors();
    }

    LOG_INFO("%s:%s:%d => thread=%d's eventloop=%p stop looping.", __FILENAME__, __FUNCTION__, __LINE__, threadId_, this);
    looping_ = false;
}

void EventLoop::quit() {
    quit_ = true;
    if(!isInLoopThread())
        wakeup();
}


Timestamp EventLoop::pollReturnTime() const {
    return pollReturnTime_;
}

void EventLoop::runInLoop(Functor cb) {
    if(isInLoopThread())
        cb();
    else
        queueInLoop(cb);
}

void EventLoop::queueInLoop(Functor cb) {
    {
        std::unique_lock<std::mutex> lock(mutex_);
        pendingFunctors_.emplace_back(cb);
    }

    if(!isInLoopThread() || callingPendingFunctors_)
        wakeup();
}

void EventLoop::wakeup() {
    uint64_t one = 1;
    ssize_t n = write(wakeupFd_, &one, sizeof one);
    if(n != sizeof one)
        LOG_ERROR("%s:%s:%d => thread=%d's eventloop=%p wakeup write %ldB, not 8B.", __FILENAME__, __FUNCTION__, __LINE__, threadId_, this, n);
}

void EventLoop::updateChannel(Channel *channel) {
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel) {
    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel *channel) {
    return poller_->hasChannel(channel);
}

bool EventLoop::isInLoopThread() const {
    return threadId_ == CurrentThread::tid();
}

void EventLoop::handleRead() {
    uint64_t one = 1;
    ssize_t n = read(wakeupFd_, &one, sizeof one);
    if(n != sizeof one)
        LOG_ERROR("%s:%s:%d => thread=%d's eventloop=%p wakeup read %ldB, not 8B", __FILENAME__, __FUNCTION__, __LINE__, threadId_, this, n);
}

void EventLoop::doPendingFunctors() {
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;

    {
        std::unique_lock<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }

    for(const Functor &functor: functors) {
        functor();
    }

    callingPendingFunctors_ = false;

}



