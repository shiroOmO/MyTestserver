#pragma once

#include "noncopyable.h"

#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <unistd.h>


class Thread: noncopyable {
public:
    using ThreadFunc = std::function<void()>;

    explicit Thread(ThreadFunc func, const std::string &name = std::string());
    ~Thread();

    void start();
    void join();

    bool started() const;
    pid_t tid() const;
    const std::string& name() const;

    static int numCreated();

private:
    void setDefaultName();

private:
    bool started_;
    bool joined_;
    std::shared_ptr<std::thread> thread_;
    pid_t tid_;
    ThreadFunc func_;
    std::string name_;
    static std::atomic_int numCreated_;
};



