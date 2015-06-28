// Copyright (c) 2015 Flash Authors. All Rights Reserved
// @author: yuanyi (yuanyi03@baidu.com)
// @brief : 

#ifndef _TIMER_H_
#define _TIMER_H_

#include <stdio.h>
#include <sys/time.h>

namespace common {
namespace util {

static inline int64_t GetMicros() {
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    return static_cast<int64_t>(now.tv_sec * 1000000 
            + now.tv_nsec / 1000);
}

static inline int32_t NowTime() {
    return static_cast<int32_t>(GetMicros() / 1000000);
}


}   // ending namespace util
}   // ending namespace common

#endif

