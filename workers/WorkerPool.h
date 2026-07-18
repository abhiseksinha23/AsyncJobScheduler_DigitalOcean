#pragma once

#include <atomic>
#include <chrono>
#include <cmath>
#include <functional>
#include <json/json.h>
#include <mutex>
#include <thread>
#include <vector>

#include "../models/Job.h"
#include "../repository/JobRepository.h"
#include "JobQueue.h"

class WorkerPool
{
public:
    using ProcessorFunc = std::function<Json::Value(const Json::Value &)>;

    WorkerPool(JobQueue &queue, JobRepository &repository, size_t numThreads,
               ProcessorFunc processor = nullptr)
        : queue_(queue), repository_(repository), processor_(std::move(processor))
    {
        scaleUp(numThreads);
    }

    ~WorkerPool()
    {
        stop();
    }

    void scaleUp(size_t count)
    {
        std::lock_guard<std::mutex> lock(workersMutex_);
        for (size_t i = 0; i < count; ++i)
        {
            workers_.emplace_back(&WorkerPool::workerLoop, this);
            activeCount_.fetch_add(1);
        }
    }

    void scaleDown(size_t count)
    {
        int current = activeCount_.load();
        if (static_cast<int>(count) > current)
            count = current;
        threadsToStop_.fetch_add(static_cast<int>(count));
    }

    int getActiveCount() const
    {
        return activeCount_.load();
    }

    void stop()
    {
        queue_.shutdown();
        std::lock_guard<std::mutex> lock(workersMutex_);
        for (auto &t : workers_)
        {
            if (t.joinable())
                t.join();
        }
        workers_.clear();
        activeCount_.store(0);
    }

private:
    void workerLoop()
    {
        while (true)
        {
            auto jobId = queue_.dequeue();
            if (!jobId)
                break;

            auto job = repository_.findById(*jobId);
            if (!job)
            {
                if (shouldStop())
                    break;
                continue;
            }

            job->setStatus(JobStatus::Running);
            job->markUpdated();
            repository_.save(*job);

            try
            {
                Json::Value result;
                if (processor_)
                {
                    result = processor_(job->getPayload());
                }
                else
                {
                    processJob(*job);
                    result["message"] = "Job processed successfully";
                }

                job->setResult(result);
                job->setStatus(JobStatus::Completed);
                job->setError("");
                job->markUpdated();
                repository_.save(*job);
            }
            catch (const TransientError &e)
            {
                handleTransientFailure(*job, e.what());
            }
            catch (const std::exception &e)
            {
                job->setStatus(JobStatus::Failed);
                job->setError(e.what());
                job->markUpdated();
                repository_.save(*job);
            }
            catch (...)
            {
                job->setStatus(JobStatus::Failed);
                job->setError("Unknown error occurred");
                job->markUpdated();
                repository_.save(*job);
            }

            if (shouldStop())
                break;
        }

        activeCount_.fetch_sub(1);
    }

    void processJob(Job &job)
    {
        const auto &payload = job.getPayload();
        bool simulateFailure = payload.isMember("simulate_failure") &&
                               payload["simulate_failure"].asBool();

        std::this_thread::sleep_for(std::chrono::seconds(2));

        if (simulateFailure && job.getRetryCount() < job.getMaxRetries())
        {
            throw TransientError("Simulated transient error");
        }
    }

    void handleTransientFailure(Job &job, const std::string &errorMsg)
    {
        if (job.getRetryCount() < job.getMaxRetries())
        {
            job.incrementRetryCount();
            job.setStatus(JobStatus::Retrying);
            job.setError(errorMsg + " (retry " + std::to_string(job.getRetryCount()) +
                         "/" + std::to_string(job.getMaxRetries()) + ")");
            job.markUpdated();
            repository_.save(job);

            int delaySecs = static_cast<int>(std::pow(2, job.getRetryCount()));
            std::this_thread::sleep_for(std::chrono::seconds(delaySecs));

            queue_.enqueue(job.getId());
        }
        else
        {
            job.setStatus(JobStatus::Failed);
            job.setError(errorMsg + " (retries exhausted)");
            job.markUpdated();
            repository_.save(job);
        }
    }

    bool shouldStop()
    {
        int expected = threadsToStop_.load();
        while (expected > 0)
        {
            if (threadsToStop_.compare_exchange_weak(expected, expected - 1))
                return true;
        }
        return false;
    }

    JobQueue &queue_;
    JobRepository &repository_;
    ProcessorFunc processor_;
    std::mutex workersMutex_;
    std::vector<std::thread> workers_;
    std::atomic<int> activeCount_{0};
    std::atomic<int> threadsToStop_{0};
};
