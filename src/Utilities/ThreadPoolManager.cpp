#include "Utilities/ThreadPoolManager.h"

std::mutex ThreadPoolManager::instance_mutex;

ThreadPoolManager::ThreadPoolManager(size_t threadCount)
    : threadPool(std::make_unique<ThreadPool>(threadCount))
{
}

ThreadPoolManager& ThreadPoolManager::GetInstance(size_t threadCount) {
    std::lock_guard<std::mutex> lock(instance_mutex);
    static ThreadPoolManager instance(threadCount);
    return instance;
}