#include "Acceptor.h"
#include "InetAddress.h"
#include "Logger.h"

#include <cerrno>
#include <functional>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>


static int createNonBlocking() {
    int sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if(sockfd < 0)
        LOG_FATAL("%s:%s:%d => listen socket fd create fail, exit, errno=%d.", __FILENAME__, __FUNCTION__, __LINE__, errno);
    return sockfd;
}

Acceptor::Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reuseport):
    loop_(loop),
    acceptSocket_(createNonBlocking()),
    acceptChannel_(loop, acceptSocket_.fd()),
    listenning_(false) {

    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(reuseport);
    acceptSocket_.bindAddress(listenAddr);

    acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor() {
    acceptChannel_.disableAll();
    acceptChannel_.remove();
}

void Acceptor::setNewConnectionCallback(const NewConnectionCallback &cb) {
    newConnectionCallback_ = cb;
}

bool Acceptor::listenning() const {
    return listenning_;
}

void Acceptor::listen() {
    listenning_ = true;
    acceptSocket_.listen();
    acceptChannel_.enableReading();
}

void Acceptor::handleRead() {
    InetAddress peerAddr;
    int connfd = acceptSocket_.accept(&peerAddr);
    if(connfd >= 0) {
        if(newConnectionCallback_)
            newConnectionCallback_(connfd, peerAddr);
        else
            close(connfd);
    }
    else {
        LOG_ERROR("%s:%s:%d => accept socket fd accept error, do not execute connection, errno=%d.", __FILENAME__, __FUNCTION__, __LINE__, errno);
        if(errno == EMFILE)
            LOG_ERROR("%s:%s:%d => socket fd reach limit, do not execute connection.", __FILENAME__, __FUNCTION__, __LINE__);
    }
}



