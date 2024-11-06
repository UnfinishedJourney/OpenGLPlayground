#pragma once

#include "Utilities/ThreadPool.h"
#include <memory>
#include <mutex>
#include <future>

class ThreadPoolManager {
public:
    static ThreadPoolManager& GetInstance(size_t threadCount = std::thread::hardware_concurrency());

    template<class F, class... Args>
    auto Enqueue(F&& f, Args&&... args)
        -> std::future<typename std::invoke_result_t<F, Args...>>
    {
        return threadPool->Enqueue(std::forward<F>(f), std::forward<Args>(args)...);
    }

private:
    explicit ThreadPoolManager(size_t threadCount);

    std::unique_ptr<ThreadPool> threadPool;

    static std::mutex instance_mutex;
};