#pragma once

#include "Timestamp.h"
#include "noncopyable.h"

#include <functional>
#include <memory>


class EventLoop;

class Channel: noncopyable {
public:
    typedef std::function<void()> EventCallback;
    using ReadEventCallback = std::function<void(Timestamp)>;

    Channel(EventLoop *loop, int fd);
    ~Channel();

    void handleEvent(Timestamp receiveTime);

    void setReadCallback(ReadEventCallback cb);
    void setWriteCallback(EventCallback cb);
    void setCloseCallback(EventCallback cb);
    void setErrorCallback(EventCallback cb);

    void tie(const std::shared_ptr<void> &obj);

    int fd() const;
    int events() const;
    void set_revents(int revt);

    void enableReading();
    void disableReading();
    void enableWriting();
    void disableWriting();
    void disableAll();

    bool isNoneEvent() const;
    bool isWriting() const;
    bool isReading() const;

    int index();
    void set_index(int idx);

    EventLoop* ownerLoop();
    void remove();

private:
    void update();
    void handleEventWithGuard(Timestamp receiveTime);

private:
    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop *loop_;
    const int fd_;
    int events_;
    int revents_;
    int index_;

    std::weak_ptr<void> tie_;
    bool tied_;

    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;
};


