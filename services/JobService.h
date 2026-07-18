#pragma once

#include <chrono>
#include <optional>
#include <string>
#include <vector>

#include "../models/Job.h"
#include "../repository/JobRepository.h"
#include "../workers/JobQueue.h"

class JobService
{
public:
    JobService(JobRepository &repository, JobQueue &queue)
        : repository_(repository), queue_(queue) {}

    Job createJob(const std::string &name, const Json::Value &payload)
    {
        Job job(generateId(), name, payload);

        repository_.save(job);
        queue_.enqueue(job.getId());

        return job;
    }

    std::optional<Job> getJob(const std::string &id) const
    {
        return repository_.findById(id);
    }

    std::vector<Job> getAllJobs() const
    {
        return repository_.findAll();
    }

    std::optional<Job> updateJob(const std::string &id, const std::string &name)
    {
        auto job = repository_.findById(id);
        if (!job)
            return std::nullopt;
        job->setName(name);
        job->markUpdated();
        repository_.save(*job);
        return job;
    }

    bool deleteJob(const std::string &id)
    {
        return repository_.remove(id);
    }

private:
    JobRepository &repository_;
    JobQueue &queue_;
    unsigned int counter_{0};

    std::string generateId()
    {
        return "job_" + std::to_string(++counter_);
    }
};
