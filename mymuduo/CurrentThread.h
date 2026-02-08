#pragma once

#include <sys/syscall.h>
#include <unistd.h>


namespace CurrentThread {
    extern __thread int t_cacheTid;

    void cacheTid();

    inline int tid() {
        if(__builtin_expect(t_cacheTid == 0, 0))
            cacheTid();
        return t_cacheTid;
    }
}
