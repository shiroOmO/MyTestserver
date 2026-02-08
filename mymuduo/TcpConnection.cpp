#include "TcpConnection.h"
#include "Callbacks.h"
#include "Channel.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "Logger.h"
#include "Socket.h"

#include <asm-generic/socket.h>
#include <cerrno>
#include <functional>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>


static EventLoop* CheckLoopNotNull(EventLoop *loop) {
    if(loop == nullptr)
        LOG_FATAL("%s:%s:%d => loop is nullptr, new TcpConnection create fail, exit", __FILENAME__, __FUNCTION__, __LINE__);
    return loop;
}

TcpConnection::TcpConnection(EventLoop *loop,
        const std::string &nameArg,
        int sockfd,
        const InetAddress &localAddr,
        const InetAddress &peerAddr):

    loop_(CheckLoopNotNull(loop)),
    name_(nameArg),
    state_(kConnecting),
    reading_(true),
    socket_(new Socket(sockfd)),
    channel_(new Channel(loop, sockfd)),
    localAddr_(localAddr),
    peerAddr_(peerAddr),
    highWaterMark_(64 * 1024 * 1024) {

    channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
    channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));

    LOG_DEBUG("%s:%s:%d => TcpConnection=%s at socket fd=%d create.", __FILENAME__, __FUNCTION__, __LINE__, name_.c_str(), sockfd);
    socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection() {
    LOG_DEBUG("%s:%s:%d => TcpConnection=%s at socket fd=%d destory, state=%d", __FILENAME__, __FUNCTION__, __LINE__, name_.c_str(), channel_->fd(), (int)state_);
}

EventLoop* TcpConnection::getLoop() const {
    return loop_;
}

const std::string& TcpConnection::name() const {
    return name_;
}

const InetAddress& TcpConnection::localAddress() const {
    return localAddr_;
}

const InetAddress& TcpConnection::peerAddress() const {
    return peerAddr_;
}

bool TcpConnection::connected() const {
    return state_ == kConnected;
}

void TcpConnection::setConnectionCallback(const ConnectionCallback& cb) {
    connectionCallback_ = cb;
}

void TcpConnection::setMessageCallback(const MessageCallback& cb) {
    messageCallback_ = cb;
}

void TcpConnection::setWriteCompleteCallback(const WriteCompleteCallback& cb) {
    writeCompleteCallback_ = cb;
}

void TcpConnection::setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark) {
    highWaterMarkCallback_ = cb;
    highWaterMark_ = highWaterMark;
}

void TcpConnection::setCloseCallback(const CloseCallback& cb) {
    closeCallback_ = cb;
}

void TcpConnection::connectEstablished() {
    setState(kConnected);
    channel_->tie(shared_from_this());
    channel_->enableReading();

    connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed() {
    if(state_ == kConnected) {
        setState(kDisconnected);
        channel_->disableAll();
        connectionCallback_(shared_from_this());
    }
    channel_->remove();
}

void TcpConnection::setState(StateE state) {
    state_ = state;
}

void TcpConnection::handleRead(Timestamp receiveTime) {
    int savedErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
    if(n > 0)
        messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
    else if(n == 0)
        handleClose();
    else {
        errno = savedErrno;
        LOG_ERROR("%s:%s:%d => data read to TcpConnection=%s at socket fd=%d's buffer fail.", __FILENAME__, __FUNCTION__, __LINE__, name_.c_str(), channel_->fd());
        handleError();
    }
}

void TcpConnection::handleWrite() {
    if(channel_->isWriting()) {
        int savedErrno = 0;
        ssize_t n = outputBuffer_.writeFd(channel_->fd(), &savedErrno);
        if(n > 0) {
            outputBuffer_.retrieve(n);
            if(outputBuffer_.readableBytes() == 0) {
                channel_->disableWriting();
                if(writeCompleteCallback_)
                    loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
                if(state_ == kDisconnecting)
                    shutdownInLoop();
            }
        }
        else
            LOG_ERROR("%s:%s:%d => data write from TcpConnection=%s at socket fd=%d's buffer fail.", __FILENAME__, __FUNCTION__, __LINE__, name_.c_str(), channel_->fd());
    }
    else
        LOG_ERROR("%s:%s:%d => TcpConnection=%s at socket fd=%d is down, no more writing.", __FILENAME__, __FUNCTION__, __LINE__, name_.c_str(), channel_->fd());
}

void TcpConnection::handleClose() {
    LOG_DEBUG("%s:%s:%d => TcpConnection=%s at socket fd=%d will close, state=%d", __FILENAME__, __FUNCTION__, __LINE__, name_.c_str(), channel_->fd(), (int)state_);
    setState(kDisconnected);
    channel_->disableAll();

    TcpConnectionPtr connPtr(shared_from_this());
    connectionCallback_(connPtr);
    closeCallback_(connPtr);
}

void TcpConnection::handleError() {
    int optval;
    socklen_t optlen = sizeof optval;
    int err = 0;
    if(getsockopt(channel_->fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
        err = errno;
    else
        err = optval;
    LOG_ERROR("%s:%s:%d => TcpConnection=%s at socket fd=%d handleError - SO_ERROR=%d.", __FILENAME__, __FUNCTION__, __LINE__, name_.c_str(), channel_->fd(), err);

}

void TcpConnection::sendInLoop(const void *data, size_t len) {
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool faultError = false;

    if(state_ == kDisconnected) {
        LOG_ERROR("%s:%s:%d => TcpConnection=%s at socket fd=%d is disconnected, giveup writing.", __FILENAME__, __FUNCTION__, __LINE__, name_.c_str(), channel_->fd());
        return ;
    }
    
    if(!channel_->isWriting() && outputBuffer_.readableBytes() == 0) {
        nwrote = write(channel_->fd(), data, len);
        if(nwrote >= 0) {
            remaining = len - nwrote;
            if(remaining == 0 && writeCompleteCallback_)
                loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
        }
        else {
            nwrote = 0;
            if(errno != EWOULDBLOCK) {
                LOG_ERROR("%s:%s:%d => TcpConnection=%s at socket fd=%d write data from user to TcpBuf fail.", __FILENAME__, __FUNCTION__, __LINE__, name_.c_str(), channel_->fd());
                if(errno == EPIPE || errno == ECONNRESET)
                    faultError = true;
            }
        }
    }

    if(!faultError && remaining > 0) {
        size_t oldLen = outputBuffer_.readableBytes();
        if(oldLen + remaining >= highWaterMark_ && oldLen < highWaterMark_ && highWaterMarkCallback_)
            loop_->queueInLoop(std::bind(highWaterMarkCallback_, shared_from_this(), oldLen + remaining));
        outputBuffer_.append((char*)data + nwrote, remaining);
        if(!channel_->isWriting())
            channel_->enableWriting();
    }
}

void TcpConnection::shutdownInLoop() {
    if(!channel_->isWriting())
        socket_->shutdownWrite();
}

void TcpConnection::send(const std::string &buf) {
    if(state_ == kConnected) {
        if(loop_->isInLoopThread())
            sendInLoop(buf.c_str(), buf.size());
        else
            loop_->runInLoop(std::bind(&TcpConnection::sendInLoop, this, buf.c_str(), buf.size()));
    }
}

void TcpConnection::send(Buffer &buf) {
    if(state_ == kConnected) {
        if(loop_->isInLoopThread())
            sendInLoop(buf.peek(), buf.readableBytes());
        else
            loop_->runInLoop(std::bind(&TcpConnection::sendInLoop, this, buf.peek(), buf.readableBytes()));
    }
}

void TcpConnection::shutdown() {
    if(state_ == kConnected) {
        setState(kDisconnecting);
        loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
    }
}



