#include "TcpServer.h"
#include "Acceptor.h"
#include "Callbacks.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"
#include "InetAddress.h"
#include "Logger.h"
#include "TcpConnection.h"

#include <cstdio>
#include <cstring>
#include <functional>
#include <string>
#include <sys/socket.h>
#include <unistd.h>


static EventLoop* CheckLoopNotNull(EventLoop *loop) {
    if(loop == nullptr)
        LOG_FATAL("%s:%s:%d => mainloop is nullptr, TcpServer create fail, exit.", __FILENAME__, __FUNCTION__, __LINE__);
    return loop;
}

TcpServer::TcpServer(EventLoop *loop, const InetAddress &listenAddr,
                        const std::string &nameArg, Option option):
    loop_(CheckLoopNotNull(loop)),
    ipPort_(listenAddr.toIpPort()),
    name_(nameArg),
    acceptor_(new Acceptor(loop, listenAddr, option == kReusePort)),
    threadPool_(new EventLoopThreadPool(loop, name_)),
    connectionCallback_(),
    messageCallback_(),
    nextConnId_(1),
    started_(0) {

    acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this,
        std::placeholders::_1, std::placeholders::_2));
}


TcpServer::~TcpServer() {
    for(auto &item: connections_) {
        TcpConnectionPtr conn(item.second);
        item.second.reset();

        conn->getLoop()->runInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
    }
}

void TcpServer::setThreadInitCallback(const ThreadInitCallback &cb) {
    threadInitCallback_ = cb;
}

void TcpServer::setConnectionCallback(const ConnectionCallback &cb) {
    connectionCallback_ = cb;
}

void TcpServer::setMessageCallback(const MessageCallback &cb) {
    messageCallback_ = cb;
}

void TcpServer::setWriteCompleteCallback(const WriteCompleteCallback &cb) {
    writeCompleteCallback_ = cb;
}

void TcpServer::setThreadNum(int numThreads) {
    threadPool_->setThreadNum(numThreads);
}

void TcpServer::start() {
    if(started_++ == 0) {
        threadPool_->start(threadInitCallback_);
        loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
    }
    
}

void TcpServer::newConnection(int sockfd, const InetAddress &peerAddr) {
    EventLoop *ioLoop = threadPool_->getNextLoop();
    char buf[64] = {0};
    snprintf(buf, sizeof buf, "-%s#%d", ipPort_.c_str(), nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;

    LOG_DEBUG("%s:%s:%d => new TcpConnection=%s at socket fd=%d from %s will create.", __FILENAME__, __FUNCTION__, __LINE__, connName.c_str(), sockfd, peerAddr.toIpPort().c_str());

    sockaddr_in local;
    memset(&local, 0, sizeof local);
    socklen_t addrlen = sizeof local;
    if(getsockname(sockfd, (sockaddr*)&local, &addrlen) < 0)
        LOG_ERROR("%s:%s:%d => new TcpConnection=%s at socket fd=%d get local address fail.", __FILENAME__, __FUNCTION__, __LINE__, connName.c_str(), sockfd);
    InetAddress localAddr(local);

    TcpConnectionPtr conn(new TcpConnection(ioLoop, connName, sockfd, localAddr, peerAddr));
    connections_[connName] = conn;
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);

    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
    
}

void TcpServer::removeConnection(const TcpConnectionPtr &conn) {
    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn) {
    LOG_DEBUG("%s:%s:%d => new TcpConnection=%s will remove", __FILENAME__, __FUNCTION__, __LINE__, conn->name().c_str());

    connections_.erase(conn->name());
    EventLoop *ioLoop = conn->getLoop();
    ioLoop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}



