#include "Buffer.h"

#include <algorithm>
#include <cerrno>
#include <string>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>


Buffer::Buffer(size_t initialSize):
    buffer_(kCheapPrepend + initialSize),
    readerIndex_(kCheapPrepend),
    writerIndex_(kCheapPrepend) {}

size_t Buffer::readableBytes() const {
    return writerIndex_ - readerIndex_;
}

size_t Buffer::writableBytes() const {
    return buffer_.size() - writerIndex_;
}

size_t Buffer::prependableBytes() const {
    return readerIndex_;
}

const char* Buffer::peek() const {
    return begin() + readerIndex_;
}

const char* Buffer::findCRLF() const {
    const char kCRLF[] = "\r\n";
    const char *crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF+2);
    return crlf == beginWrite() ? nullptr : crlf;
}

void Buffer::retrieve(size_t len) {
    if(len < readableBytes())
        readerIndex_ += len;
    else
        retrieveAll();
}

void Buffer::retrieveAll() {
    readerIndex_ = writerIndex_ = kCheapPrepend;
}

std::string Buffer::retrieveAsString(size_t len) {
    std::string result(peek(), len);
    retrieve(len);
    return result;
}

std::string Buffer::retrieveAllAsString() {
    return retrieveAsString(readableBytes());
}

void Buffer::ensureWriteableBytes(size_t len) {
    if(writableBytes() < len)
        makeSpace(len);
}

void Buffer::append(const char *data, size_t len) {
    ensureWriteableBytes(len);
    std::copy(data, data + len, beginWrite());
    writerIndex_ += len;
}

char* Buffer::beginWrite() {
    return begin() + writerIndex_;
}

const char* Buffer::beginWrite() const {
    return begin() + writerIndex_;
}

ssize_t Buffer::readFd(int fd, int *saveErrno) {
    char extrabuf[65536] = {0};

    struct iovec vec[2];
    const size_t writable = writableBytes();
    vec[0].iov_base = begin() + writerIndex_;
    vec[0].iov_len = writable;

    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof extrabuf;

    const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;
    const ssize_t n = readv(fd, vec, iovcnt);

    if(n < 0)
        *saveErrno = errno;
    else if(n <= writable)
        writerIndex_ += n;
    else {
        writerIndex_ = buffer_.size();
        append(extrabuf, n - writable);
    }

    return n;
}

ssize_t Buffer::writeFd(int fd, int *saveErrno) {
    ssize_t n = write(fd, peek(), readableBytes());
    if(n < 0)
        *saveErrno = errno;
    return n;
}

char* Buffer::begin() {
    return &*buffer_.begin();
}

const char* Buffer::begin() const {
    return &*buffer_.begin();
}

void Buffer::makeSpace(size_t len) {
    if(writableBytes() + prependableBytes() - kCheapPrepend < len)
        buffer_.resize(writerIndex_ + len);
    else {
        size_t readable = readableBytes();
        std::copy(begin() + readerIndex_, begin() + writerIndex_, begin() + kCheapPrepend);
        readerIndex_ = kCheapPrepend;
        writerIndex_ = readerIndex_ + readable;
    }
}



