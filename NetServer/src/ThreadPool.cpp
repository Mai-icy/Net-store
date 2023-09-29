#include "ThreadPool.h"

ThreadPool::ThreadPool(int numThreads) : stop(false) {
    pthread_mutex_init(&queueMutex, nullptr);
    pthread_cond_init(&condition, nullptr);

    for (int i = 0; i < numThreads; ++i) {
        pthread_t tid;
        pthread_create(&tid, nullptr, workerThread, this);
        threads.push_back(tid);
    }
}

ThreadPool::~ThreadPool() {
    {
        pthread_mutex_lock(&queueMutex);
        stop = true;
        pthread_mutex_unlock(&queueMutex);
    }
    pthread_cond_broadcast(&condition);

    for (pthread_t tid : threads) {
        pthread_join(tid, nullptr);
    }

    pthread_mutex_destroy(&queueMutex);
    pthread_cond_destroy(&condition);
}

void ThreadPool::enqueue(std::function<void(int)> task, int arg) {
    pthread_mutex_lock(&queueMutex);
    if (stop) {
        throw std::runtime_error("enqueue on stopped ThreadPool");
    }
    tasks.push({task, arg});
    pthread_mutex_unlock(&queueMutex);
    pthread_cond_signal(&condition);
}

void* ThreadPool::workerThread(void* arg) {
    ThreadPool* pool = static_cast<ThreadPool*>(arg);
    while (true) {
        pthread_mutex_lock(&pool->queueMutex);
        while (pool->tasks.empty() && !pool->stop) {
            pthread_cond_wait(&pool->condition, &pool->queueMutex);
        }
        if (pool->stop && pool->tasks.empty()) {
            pthread_mutex_unlock(&pool->queueMutex);
            return nullptr;
        }
        auto task = pool->tasks.front();
        pool->tasks.pop();
        pthread_mutex_unlock(&pool->queueMutex);
        task.function(task.arg);
    }
}

