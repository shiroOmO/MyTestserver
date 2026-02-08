#pragma once

#include "Buffer.h"
#include "Callbacks.h"
#include "InetAddress.h"
#include "noncopyable.h"
#include "Timestamp.h"

#include <atomic>
#include <memory>
#include <string>


class Channel;
class EventLoop;
class Socket;

class TcpConnection: noncopyable, public std::enable_shared_from_this<TcpConnection> {
public:
    TcpConnection(EventLoop *loop,
            const std::string &nameArg,
            int sockfd,
            const InetAddress &localAddr,
            const InetAddress &peerAddr);
    ~TcpConnection();

    EventLoop* getLoop() const;
    const std::string& name() const;
    const InetAddress& localAddress() const;
    const InetAddress& peerAddress() const;

    bool connected() const;

    void setConnectionCallback(const ConnectionCallback& cb);
    void setMessageCallback(const MessageCallback& cb);
    void setWriteCompleteCallback(const WriteCompleteCallback& cb);
    void setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark);
    void setCloseCallback(const CloseCallback& cb);

    void connectEstablished();
    void connectDestroyed();

    void send(const std::string &buf);
    void send(Buffer &buf);

    void shutdown();

private:
    enum StateE {kDisconnected, kDisconnecting, kConnected, kConnecting};
    void setState(StateE state);

    void handleRead(Timestamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();

    void sendInLoop(const void *data, size_t len);
    void shutdownInLoop();

private:
    EventLoop *loop_;
    const std::string name_;
    std::atomic_int state_;
    bool reading_;

    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;

    const InetAddress localAddr_;
    const InetAddress peerAddr_;

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    HighWaterMarkCallback highWaterMarkCallback_;
    CloseCallback closeCallback_;
    size_t highWaterMark_;

    Buffer inputBuffer_;
    Buffer outputBuffer_;
};



