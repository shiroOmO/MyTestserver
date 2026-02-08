#pragma once

#include "Acceptor.h"
#include "Callbacks.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"
#include "InetAddress.h"
#include "Logger.h"
#include "noncopyable.h"
#include "TcpConnection.h"
#include "Timestamp.h"

#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>


class TcpServer: noncopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;
private:
    using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;

public:
    enum Option{
        kNoReusePort,
        kReusePort
    };

    TcpServer(EventLoop *loop, const InetAddress &listenAddr, const std::string &nameArg, Option option = kNoReusePort);
    ~TcpServer();

    void setThreadInitCallback(const ThreadInitCallback &cb);
    void setConnectionCallback(const ConnectionCallback &cb);
    void setMessageCallback(const MessageCallback &cb);
    void setWriteCompleteCallback(const WriteCompleteCallback &cb);

    void setThreadNum(int numThreads);

    void start();

private:
    void newConnection(int sockfd, const InetAddress &peerAddr);
    void removeConnection(const TcpConnectionPtr &conn);
    void removeConnectionInLoop(const TcpConnectionPtr &conn);

private:
    EventLoop *loop_;
    const std::string ipPort_;
    const std::string name_;
    std::unique_ptr<Acceptor> acceptor_;
    std::shared_ptr<EventLoopThreadPool> threadPool_;

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;

    ThreadInitCallback threadInitCallback_;
    std::atomic_int started_;

    int nextConnId_;
    ConnectionMap connections_;
};



