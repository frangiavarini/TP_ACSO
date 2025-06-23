

#ifndef _thread_pool_
#define _thread_pool_

#include <cstddef>
#include <functional>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include "Semaphore.h"

using namespace std;



typedef struct worker {
    thread ts;
    function<void(void)> thunk;
    Semaphore sem;      
    bool available = true;

    worker() : sem(0) {} 
} worker_t;

class ThreadPool {
  public:
    ThreadPool(size_t numThreads);
    void schedule(const function<void(void)>& thunk);
    void wait();
    ~ThreadPool();

  private:
    void dispatcher();     
    void worker(int id);  

    thread dt;
    vector<worker_t> wts;
    queue<function<void(void)>> taskQueue;
    mutex queueLock;
    condition_variable dispatcherCV;

    mutex waitLock;
    condition_variable waitCV;

    size_t runningTasks = 0;
    Semaphore availableWorkers;
    bool done = false;
};

#endif



