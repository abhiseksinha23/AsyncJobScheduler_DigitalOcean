#pragma once

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>
#include <string>

class JobQueue
{
public:
    void enqueue(const std::string &jobId)
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push(jobId);
        }
        cv_.notify_one();
    }

    std::optional<std::string> dequeue()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this] { return !queue_.empty() || shutdown_; });

        if (shutdown_ && queue_.empty())
            return std::nullopt;

        std::string jobId = queue_.front();
        queue_.pop();
        return jobId;
    }

    void shutdown()
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            shutdown_ = true;
        }
        cv_.notify_all();
    }

    bool isShutdown() const
    {
        return shutdown_;
    }

private:
    std::queue<std::string> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool shutdown_{false};
};
