// Copyriht (c) 2015, Flash Authors. All Rights Reserved
// Author: yuanyi03@baidu.com

#ifndef _THREAD_SPIN_H
#define _THREAD_SPIN_H

#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "common/util/noncopyable.h"
#include "common/util/log.h"
#include "common/util/time_debuger.h"

namespace common {
namespace thread {

class Spin : public common::util::noncopyable {
public:
    Spin() : _spin_lock() {
        // pshared options is private 
        int err = pthread_spin_init(&_spin_lock, PTHREAD_PROCESS_PRIVATE); 
        if (err != 0) {
            FATAL_LOG(
                    "thread[%x] spinlock[%p] init failed err[%d] [%s]",
                    pthread_self(),
                    &_spin_lock,
                    errno,
                    strerror(errno)); 
            abort();
        }
    }

    ~Spin() {
        int err = pthread_spin_destroy(&_spin_lock); 
        if (err != 0) {
            FATAL_LOG(
                    "thread[%x] spinlock[%p] destroy failed err[%d] [%s]",
                    pthread_self(),
                    &_spin_lock,
                    errno,
                    strerror(errno)); 
        }
    }

    void Lock() {
#ifdef UTTEST
        common::util::TimerDebuger timer(
                "thread[%x] spinlock[%p] lock costs ", 
                pthread_self(),
                &_spin_lock);
#endif
        int err = pthread_spin_lock(&_spin_lock);
        if (err != 0) {
            FATAL_LOG(
                    "thread[%x] spinlock[%p] lock failed err[%d] [%s]",
                    pthread_self(),
                    &_spin_lock,
                    errno,
                    strerror(errno)); 
            abort();
        }
    }

    void UnLock() {
#ifdef UTTEST
        common::util::TimerDebuger timer(
                "thread[%x] spinlock[%p] unlock costs ",
                pthread_self(),
                &_spin_lock);
#endif
        int err = pthread_spin_unlock(&_spin_lock); 
        if (err != 0) {
            FATAL_LOG(
                    "thread[%x] spinlock[%p] unlock failed err[%d] [%s]",
                    pthread_self(),
                    &_spin_lock,
                    errno,
                    strerror(errno)); 
            abort();
        }
    }
private:
    pthread_spin_t _spin_lock;    
};
}   // ending namespace thread
}   // ending namespace common

#endif  //PS_SPI_SRC/COMMON/THREAD/SPIN_H

/* vim: set ts=4 sw=4 sts=4 tw=100 */
