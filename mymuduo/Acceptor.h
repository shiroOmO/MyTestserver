#pragma once

#include "Channel.h"
#include "noncopyable.h"
#include "Socket.h"

#include <functional>


class InetAddress;
class EventLoop;

class Acceptor: noncopyable {
public:
    using NewConnectionCallback = std::function<void(int, const InetAddress&)>;

    Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reuseport);
    ~Acceptor();

    void setNewConnectionCallback(const NewConnectionCallback &cb);

    bool listenning() const;
    void listen();

private:
    void handleRead();

private:
    EventLoop *loop_;
    Socket acceptSocket_;  // listen_fd
    Channel acceptChannel_;
    NewConnectionCallback newConnectionCallback_;
    bool listenning_;
};




