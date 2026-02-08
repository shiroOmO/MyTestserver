#pragma once

#include <cstddef>
#include <string>
#include <sys/types.h>
#include <vector>


class Buffer {
public:
    explicit Buffer(size_t initialSize = kInitialSize);

    size_t readableBytes() const;
    size_t writableBytes() const;
    size_t prependableBytes() const;

    const char* peek() const;
    const char* findCRLF() const;

    void retrieve(size_t len);
    void retrieveAll();

    std::string retrieveAsString(size_t len);
    std::string retrieveAllAsString();

    void ensureWriteableBytes(size_t len);

    void append(const char *data, size_t len);
    char* beginWrite();
    const char* beginWrite() const;

    ssize_t readFd(int fd, int *saveErrno);
    ssize_t writeFd(int fd, int *saveErrno);

private:
    char* begin();
    const char* begin() const;
    void makeSpace(size_t len);

public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;

private:
    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;
};


