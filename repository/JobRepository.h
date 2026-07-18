#pragma once

#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "../models/Job.h"

class JobRepository
{
public:
    void save(const Job &job)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        jobs_[job.getId()] = job;
    }

    std::optional<Job> findById(const std::string &id) const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = jobs_.find(id);
        if (it == jobs_.end())
            return std::nullopt;
        return it->second;
    }

    std::vector<Job> findAll() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<Job> result;
        result.reserve(jobs_.size());
        for (const auto &[_, job] : jobs_)
            result.push_back(job);
        return result;
    }

    bool remove(const std::string &id)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return jobs_.erase(id) > 0;
    }

    bool exists(const std::string &id) const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return jobs_.find(id) != jobs_.end();
    }

private:
    mutable std::mutex mutex_;
    std::unordered_map<std::string, Job> jobs_;
};
