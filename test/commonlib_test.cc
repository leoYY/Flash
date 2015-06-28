// Copyright (c) 2015 Flash Authors. All Rights Reserved
// @author: yuanyi (yuanyi03@baidu.com)
// @brief : 

#include "common/thread/thread_pool.h"
#include "common/util/log.h"

using common::thread::ThreadPool;

int main(int argc, char* argv[]) {
    LOG_INIT();
    ThreadPool pool(10);
    if (!pool.Start()) {
        FATAL_LOG("thread pool start failed");
        LOG_UNINIT();
        return -1;
    }
    LOG_UNINIT(); 
    return 0;
}

