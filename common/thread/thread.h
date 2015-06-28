// Copyright (c) 2015 Flash Authors. All Rights Reserved
// @author: yuanyi (yuanyi03@baidu.com)
// @brief : 

#ifndef _THREAD_H_
#define _THREAD_H_

#include <pthread.h>
#include <boost/function.hpp>

namespace common {
namespace util {

class Thread {
public:
    bool Start(boost::function<void()> thread_proc) {
        user_proc_ = thread_proc; 
        int ret = pthread_create(&tid_, NULL, ProcWrapper, this);
        return (ret == 0);
    }

    bool Join() {
        int ret = pthread_join(tid_, NULL); 
        return (ret == 0);
    }
private:
    static void* ProcWrapper(void* arg) {
        reinterpret_cast<Thread*>(arg)->user_proc_();
        return NULL;
    }
    boost::function<void()> user_proc_;
    pthread_t tid_;
};

}   // ending namespace util
}   // ending namespace common

#endif

