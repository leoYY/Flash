// Copyright (c) 2015 Flash Authors. All Rights Reserved
// @author: yuanyi (yuanyi03@baidu.com)
// @brief : 

#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_


#include <deque>
#include <map>
#include <queue>
#include "common/util/noncopyable.h"
#include "common/thread/thread.h"
#include "common/thread/mutex.h"
#include "common/thread/condition.h"

namespace common {
namespace thread {

struct TimeTask;

class ThreadPool : public common::util::noncopyable {
public:
    ThreadPool(int thread_num = 10);
    ~ThreadPool();
    typedef boost::function<void()> Task;

    bool Start();
    bool Stop(bool wait = false);
    void AddTask(const Task& task);
    void AddPriorityTask(const Task& task);
    int64_t AddTimeTask(const Task& task, int64_t delay_times);
    bool CancelTimeTask(int64_t task_id);

    int64_t PendingNum() const {
        return pending_num_;
    }
private:
    bool GetTask(Task& task);
    void ThreadProc();

    static void* TaskProcWrapper(void* arg) {
        reinterpret_cast<ThreadPool*>(arg)->ThreadProc(); 
        return NULL;
    }

    struct TimeTask {
        int64_t id; 
        int64_t exe_time;
        Task task;
        bool canceled;
        bool operator<(const TimeTask& task) const {
            if (exe_time != task.exe_time) {
                return exe_time > task.exe_time ;
            } 
            return id > task.id;
        }
    };


    std::deque<Task> normal_task_queue_;
    volatile int pending_num_;
    int thread_num_;
    Mutex mutex_;
    Condition cond_;
    std::vector<pthread_t> tids_;

    std::map<int64_t, TimeTask> time_tasks_;
    std::priority_queue<TimeTask> time_task_queue_;
    int64_t next_time_task_id_;
    volatile bool stop_; 
};

}   // ending namespace thread
}   // ending namespace common

#endif

