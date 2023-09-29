#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <iostream>
#include <vector>
#include <queue>
#include <pthread.h>
#include <functional>


class ThreadPool {
public:
    ThreadPool(int numThreads);
    ~ThreadPool();

    void enqueue(std::function<void(int)> func, int arg);

private:
    static void* workerThread(void* arg);

private:
    struct Task {
        std::function<void(int)> function;
        int arg;
    };

    std::vector<pthread_t> threads;
    std::queue<Task> tasks;
    pthread_mutex_t queueMutex;
    pthread_cond_t condition;
    bool stop;
};

#endif
