#include "Channel.h"
#include "EventLoop.h"
#include "Timestamp.h"
#include "Logger.h"

#include <memory>
#include <sys/epoll.h>


const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;

Channel::Channel(EventLoop *loop, int fd):
    loop_(loop), fd_(fd), events_(0), revents_(0), index_(-1), tied_(false) {}

Channel::~Channel() {}

void Channel::handleEvent(Timestamp receiveTime) {
    if(tied_) {
        std::shared_ptr<void> guard = tie_.lock();
        if(guard)
            handleEventWithGuard(receiveTime);
    }
    else
        handleEventWithGuard(receiveTime);
}

void Channel::setReadCallback(ReadEventCallback cb) {
    readCallback_ = std::move(cb);
}

void Channel::setWriteCallback(EventCallback cb) {
    writeCallback_ = std::move(cb);
}

void Channel::setCloseCallback(EventCallback cb) {
    closeCallback_ = std::move(cb);
}

void Channel::setErrorCallback(EventCallback cb) {
    errorCallback_ = std::move(cb);
}

void Channel::tie(const std::shared_ptr<void> &obj) {
    tie_ = obj;
    tied_ = true;
}

int Channel::fd() const {
    return fd_;
}

int Channel::events() const {
    return events_;
}

void Channel::set_revents(int revt) {
    revents_ = revt;
}

void Channel::enableReading() {
    events_ |= kReadEvent;
    update();
}

void Channel::disableReading() {
    events_ &= ~kReadEvent;
    update();
}

void Channel::enableWriting() {
    events_ |= kWriteEvent;
    update();
}

void Channel::disableWriting() {
    events_ &= ~kWriteEvent;
    update();
}

void Channel::disableAll() {
    events_ = kNoneEvent;
    update();
}

bool Channel::isNoneEvent() const {
    return events_ == kNoneEvent;
}

bool Channel::isWriting() const {
    return events_ & kWriteEvent;
}

bool Channel::isReading() const {
    return events_ & kReadEvent;
}

int Channel::index() {
    return index_;
}

void Channel::set_index(int idx) {
    index_ = idx;
}

EventLoop* Channel::ownerLoop() {
    return loop_;
}

void Channel::remove() {
    loop_->removeChannel(this);
}

void Channel::update() {
    loop_->updateChannel(this);
}

void Channel::handleEventWithGuard(Timestamp receiveTime) {
    LOG_DEBUG("%s:%s:%d => socket fd=%d will handle event, revents_=%d.", __FILENAME__, __FUNCTION__, __LINE__, fd_, revents_);

    if((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {
        if(closeCallback_)
            closeCallback_();
    }

    if(revents_ & EPOLLERR) {
        if(errorCallback_)
            errorCallback_();
    }

    if(revents_ & (EPOLLIN | EPOLLPRI)) {
        if(readCallback_)
            readCallback_(receiveTime);
    }

    if(revents_ & EPOLLOUT) {
        if(writeCallback_)
            writeCallback_();
    }
}





