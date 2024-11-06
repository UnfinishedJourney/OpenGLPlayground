#include "Utilities/ThreadPool.h"

ThreadPool::ThreadPool(size_t numThreads) {
    for (size_t i = 0; i < numThreads; ++i) {
        m_Threads.emplace_back([this](std::stop_token stopToken) { ThreadLoop(stopToken); });
    }
}

ThreadPool::~ThreadPool() {
    // std::jthread automatically joins threads upon destruction
}

void ThreadPool::ThreadLoop(std::stop_token stopToken) {
    while (!stopToken.stop_requested()) {
        std::function<void()> task;
        {
            std::unique_lock lock(m_Mutex);
            m_Condition.wait(lock, [this, &stopToken]() { return !m_Tasks.empty() || stopToken.stop_requested(); });
            if (stopToken.stop_requested() && m_Tasks.empty())
                return;
            if (!m_Tasks.empty()) {
                task = std::move(m_Tasks.front());
                m_Tasks.pop();
            }
        }
        if (task)
            task();
    }
}