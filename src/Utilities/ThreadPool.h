#pragma once

#include <vector>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <stop_token>

class ThreadPool {
public:
    explicit ThreadPool(size_t numThreads);
    ~ThreadPool();

    // Enqueue a task
    template <class F>
    void Enqueue(F&& task);

private:
    std::vector<std::jthread> m_Threads;
    std::queue<std::function<void()>> m_Tasks;

    std::mutex m_Mutex;
    std::condition_variable_any m_Condition;

    void ThreadLoop(std::stop_token stopToken);
};

template <class F>
void ThreadPool::Enqueue(F&& task) {
    {
        std::lock_guard lock(m_Mutex);
        m_Tasks.emplace(std::forward<F>(task));
    }
    m_Condition.notify_one();
}