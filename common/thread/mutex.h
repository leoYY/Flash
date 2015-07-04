// Copyriht (c) 2015, Flash Authors. All Rights Reserved
// Author: yuanyi03@baidu.com
#ifndef _THREAD_MUTEX_H
#define _THREAD_MUTEX_H

#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include "common/util/noncopyable.h"
#include "common/util/log.h"
#include "common/util/time_debuger.h"

namespace common {
namespace thread {

class Mutex : public common::util::noncopyable {
public:
    Mutex()  {
        int ret = pthread_mutex_init(&_mutex, NULL);
        if (ret != 0) {
            FATAL_LOG(
                    "%d pthread mutex init failed errno[%d] [%s]\n", 
                    pthread_self(), 
                    errno, 
                    strerror(errno));     
            assert(ret == 0);
        }
    }

    ~Mutex() {
        if (0 != pthread_mutex_destroy(&_mutex)) {
            FATAL_LOG(
                    "%d pthread mutex destroy failed errno[%d] [%s]\n", 
                    pthread_self(), 
                    errno, 
                    strerror(errno)); 
        }

    }
    
    void Lock(const char* wait_info = NULL) {
#ifdef UTTEST
        {
            common::util::TimerDebuger timer(
                    "thread [%x] mutex[%p] wait costs ", 
                    pthread_self(), 
                    &_mutex);
            if (wait_info != NULL) {
                timer.ResetInfo(wait_info); 
            }
#endif
        NOT_USED(wait_info);
        int err = pthread_mutex_lock(&_mutex); 
        if (err != 0) {
            FATAL_LOG(
                    "%x mutex[%p] lock failed err[%d] [%s]", 
                    pthread_self(), 
                    &_mutex, 
                    errno, 
                    strerror(errno)); 
            abort();
        } 
#ifdef UTTEST
        }
#endif 
    }

    void UnLock() {
        int err = pthread_mutex_unlock(&_mutex); 
        if (err != 0) {
            FATAL_LOG(
                    "%x mutex[%p] unlock failed err[%d] [%s]", 
                    pthread_self(),
                    &_mutex,
                    errno,
                    strerror(errno)); 
            abort();
        }
    }

    pthread_mutex_t* PthreadMutex() {
        return &_mutex; 
    }
private:
    pthread_mutex_t _mutex;
};

class MutexScoped : public common::util::noncopyable {
public:
    MutexScoped(Mutex& mutex, const char* locked_info = NULL)
        : _mutex(mutex), _locked_timer(NULL) {
        _mutex.Lock();     
#ifdef UTTEST
        _locked_timer = new common::util::TimerDebuger(
                "thread [%x] mutex[%p] locked costs",
                pthread_self(),
                _mutex.PthreadMutex());
        if (locked_info != NULL) {
            _locked_timer->ResetInfo(locked_info);
        }
#endif
        NOT_USED(locked_info);
    }

    ~MutexScoped() {
        if (_locked_timer != NULL) {
            delete _locked_timer; 
            _locked_timer = NULL;
        } 
        _mutex.UnLock(); 
    }
private:
    Mutex& _mutex;
    common::util::TimerDebuger* _locked_timer;
};


}   // ending namespace thread
}   // ending namespace common
#endif  //PS_SPI_THREAD/MUTEX_H

/* vim: set ts=4 sw=4 sts=4 tw=100 */
