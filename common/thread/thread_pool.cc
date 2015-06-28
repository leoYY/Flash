// Copyright (c) 2015 Flash Authors. All Rights Reserved
// @author: yuanyi (yuanyi03@baidu.com)
// @brief : 

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "common/thread/thread_pool.h"
#include "common/util/log.h"
#include "common/util/timer.h"

namespace common {
namespace thread {

static int DEFAULT_WAIT_TIME = 100;

ThreadPool::ThreadPool(int thread_num)
    : normal_task_queue_(),
      pending_num_(0),
      thread_num_(thread_num),
      mutex_(),
      cond_(mutex_),
      tids_(),
      time_tasks_(),
      time_task_queue_(),
      next_time_task_id_(0),
      stop_(false) {
}

ThreadPool::~ThreadPool() {
    Stop(false);
}

bool ThreadPool::Start() {
    MutexScoped lock(mutex_, "Start Pool");
    for (int i = 0; i < thread_num_; i++) {
        pthread_t tid; 
        int ret = pthread_create(&tid, NULL, 
                ThreadPool::TaskProcWrapper, this);
        if (ret != 0) {
            FATAL_LOG("thread start failed for "
                    "pthread_create fail err[%d: %s]", 
                    errno, strerror(errno));        
            return false;
        }
        tids_.push_back(tid);
    }
    return true;
}

void ThreadPool::AddTask(const Task& task) {
    MutexScoped lock(mutex_);
    pending_num_ ++;
    normal_task_queue_.push_back(task); 
    cond_.Signal();
    return;
}

void ThreadPool::AddPriorityTask(const Task& task) {
    MutexScoped lock(mutex_);
    pending_num_ ++;
    normal_task_queue_.push_front(task);
    cond_.Signal();
    return;
}

int64_t ThreadPool::AddTimeTask(const Task& task, 
        int64_t delay_times) {
    MutexScoped lock(mutex_);
    TimeTask time_task;
    time_task.id = next_time_task_id_++;
    time_task.exe_time = util::NowTime() + delay_times;
    time_task.task = task;
    time_task.canceled = false;
    time_tasks_[time_task.id] = time_task;
    time_task_queue_.push(time_task);
    pending_num_ ++;
    cond_.Signal();
    return time_task.id;
}

bool ThreadPool::CancelTimeTask(int64_t task_id) {
    MutexScoped lock(mutex_);
    std::map<int64_t, TimeTask>::iterator it
        = time_tasks_.find(task_id);
    if (it == time_tasks_.end()) {
        WARNING_LOG("task id[%ld] not exists", task_id);
        return false;
    }

    it->second.canceled = true;
    return true;
}

bool ThreadPool::GetTask(Task& task) {
    while (!stop_) {
        MutexScoped lock(mutex_, "Get task costs"); 
        int64_t wait_time = DEFAULT_WAIT_TIME;
        bool need_do_task = false;
        // wait for lock, may stoped
        if (stop_) {
            break; 
        }
        // check time task first
        if (!time_task_queue_.empty()) {
            TimeTask time_task = time_task_queue_.top();  
            int32_t now_time = util::NowTime();
            if (time_task.exe_time >= now_time) {
                time_tasks_.erase(time_task.id);
                time_task_queue_.pop();
                if (!time_task.canceled) {
                    task = time_task.task; 
                    need_do_task = true;                      
                } else {
                    // maybe next time_task need done
                    continue; 
                }
            } else {
                wait_time = now_time - time_task.exe_time;     
                if (wait_time > DEFAULT_WAIT_TIME) {
                    wait_time = DEFAULT_WAIT_TIME; 
                }
            }
        } 
        // check normal task second
        if (!need_do_task
                && !normal_task_queue_.empty()) {
            task = normal_task_queue_.front();        
            normal_task_queue_.pop_front();
            need_do_task = true;
        }

        if (need_do_task) {
            pending_num_ --;
            return true; 
        }

        // not use Wait() for signal lose
        cond_.WaitForTime(wait_time, "Wait time task"); 
    }
    return false;
}

void ThreadPool::ThreadProc() {
    while (!stop_) {
        Task task;
        if (GetTask(task)) {
            task();     
        }  
    }
    return; 
}

bool ThreadPool::Stop(bool wait) {
    if (wait) {
        while (pending_num_ > 0) {
            usleep(1000);
        } 
    }

    stop_ = true;
    cond_.SignalAll();
    std::vector<pthread_t>::iterator it 
        = tids_.begin();
    for (; it != tids_.end(); ++it) {
        pthread_join(*it, NULL); 
    }
    tids_.clear();
    return true;
}


}   // ending namespace thread
}   // ending namespace common
