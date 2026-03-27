#include "Socket.h"
#include "InetAddress.h"
#include "Logger.h"

#include <cstring>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>


Socket::Socket(int sockfd): sockfd_(sockfd) {}

Socket::~Socket() {
    close(sockfd_);
}

int Socket::fd() const {
    return sockfd_;
}

void Socket::bindAddress(const InetAddress &localaddr) {
    if(bind(sockfd_, (sockaddr*)localaddr.getSockAddr(), sizeof(sockaddr_in)) != 0)
        LOG_FATAL("listen socket fd bind address fail, exit.");
}

void Socket::listen() {
    if(::listen(sockfd_, 1024) != 0)
        LOG_FATAL("listen socket fd listen fail, exit.");
}


int Socket::accept(InetAddress *peerAddr) {
    sockaddr_in addr;
    socklen_t len = sizeof addr;
    memset(&addr, 0, sizeof addr);
    int connfd = accept4(sockfd_, (sockaddr*)&addr, &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if(connfd >= 0)
        peerAddr->setSockAddr(addr);
    return connfd;
}

void Socket::shutdownWrite() {
    if(shutdown(sockfd_, SHUT_WR) < 0)
        LOG_ERROR("socket fd=%d shutdown write fail.", sockfd_);
}

void Socket::setTcpNoDelay(bool on) {
    int optval = on ? 1 : 0;
    setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof optval);
}

void Socket::setReuseAddr(bool on) {
    int optval = on ? 1 : 0;
    setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
}

void Socket::setReusePort(bool on) {
    int optval = on ? 1 : 0;
    setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof optval);
}

void Socket::setKeepAlive(bool on) {
    int optval = on ? 1 : 0;
    setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof optval);
}



