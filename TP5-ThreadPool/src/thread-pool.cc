

#include "thread-pool.h"
using namespace std;

ThreadPool::ThreadPool(size_t numThreads) 
    : wts(numThreads), availableWorkers(0), done(false) {

    for (size_t i = 0; i < numThreads; ++i) {
        wts[i].ts = thread([this, i] { worker(i); });
        availableWorkers.signal(); 
    }

    dt = thread([this] { dispatcher(); });
}



void ThreadPool::schedule(const function<void(void)>& thunk) {
    {
        lock_guard<mutex> lock(queueLock);
        if (done) return;  
        taskQueue.push(thunk);
    }
    dispatcherCV.notify_one();
}

void ThreadPool::dispatcher() {
    while (true) {
        function<void(void)> thunk;
        {
            unique_lock<mutex> lock(queueLock);
            dispatcherCV.wait(lock, [this]() {
                return done || !taskQueue.empty();
            });

            if (done && taskQueue.empty()) break;

            thunk = taskQueue.front();
            taskQueue.pop();
        }

        availableWorkers.wait(); 

        for (size_t i = 0; i < wts.size(); ++i) {
            if (wts[i].available) {
                wts[i].available = false;
                wts[i].thunk = thunk;
                {
                    lock_guard<mutex> guard(waitLock);
                    runningTasks++;
                }
                wts[i].sem.signal(); 
                break;
            }
        }
    }
}

void ThreadPool::worker(int id) {
    while (true) {
        wts[id].sem.wait();

        if (done && wts[id].thunk == nullptr) break;

        wts[id].thunk();

        wts[id].thunk = nullptr;
        wts[id].available = true;

        {
            lock_guard<mutex> guard(waitLock);
            runningTasks--;
            if (runningTasks == 0 && taskQueue.empty()) {
                waitCV.notify_all();
            }
        }

        availableWorkers.signal();
    }
}

void ThreadPool::wait() {
    unique_lock<mutex> lock(waitLock);
    waitCV.wait(lock, [this]() {
        return runningTasks == 0 && taskQueue.empty();
    });
}

ThreadPool::~ThreadPool() {
    wait();

    done = true;
    dispatcherCV.notify_all();

    for (size_t i = 0; i < wts.size(); ++i) {
        wts[i].sem.signal();  
    }

    if (dt.joinable()) dt.join();

    for (auto& w : wts) {
        if (w.ts.joinable()) w.ts.join();
    }
}



