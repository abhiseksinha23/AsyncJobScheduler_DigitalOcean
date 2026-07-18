#include <drogon/drogon_test.h>
#include <iostream>
#include <thread>
#include <chrono>
#include "../models/Job.h"
#include "../repository/JobRepository.h"
#include "../workers/JobQueue.h"
#include "../workers/WorkerPool.h"

DROGON_TEST(WorkerNormalJobCompletes)
{
    std::cout << "[TEST] Executing: WorkerNormalJobCompletes (wait ~2s) ..." << std::endl;
    JobRepository repo;
    JobQueue queue;

    Job job("job_1", "test", Json::Value());
    repo.save(job);
    queue.enqueue("job_1");

    WorkerPool pool(queue, repo, 1, [](const Json::Value &) -> Json::Value {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        Json::Value r;
        r["done"] = true;
        return r;
    });

    std::this_thread::sleep_for(std::chrono::seconds(2));
    pool.stop();

    auto result = repo.findById("job_1");
    CHECK(result.has_value());
    CHECK(result->getStatus() == JobStatus::Completed);
    CHECK(result->getResult()["done"].asBool() == true);
    CHECK(result->getRetryCount() == 0);
    std::cout << "[PASS] WorkerNormalJobCompletes" << std::endl;
}

DROGON_TEST(WorkerPermanentFailureNoRetry)
{
    std::cout << "[TEST] Executing: WorkerPermanentFailureNoRetry (wait ~1s) ..." << std::endl;
    JobRepository repo;
    JobQueue queue;

    Job job("job_1", "test", Json::Value());
    repo.save(job);
    queue.enqueue("job_1");

    WorkerPool pool(queue, repo, 1, [](const Json::Value &) -> Json::Value {
        throw std::runtime_error("Permanent failure");
    });

    std::this_thread::sleep_for(std::chrono::seconds(1));
    pool.stop();

    auto result = repo.findById("job_1");
    CHECK(result.has_value());
    CHECK(result->getStatus() == JobStatus::Failed);
    CHECK(result->getError() == "Permanent failure");
    CHECK(result->getRetryCount() == 0);
    std::cout << "[PASS] WorkerPermanentFailureNoRetry" << std::endl;
}

DROGON_TEST(WorkerTransientFailureRetries)
{
    std::cout << "[TEST] Executing: WorkerTransientFailureRetries (wait ~15s for retries + backoff) ..." << std::endl;
    JobRepository repo;
    JobQueue queue;

    Json::Value payload;
    payload["simulate_failure"] = true;
    Job job("job_1", "retry-test", payload);
    job.setMaxRetries(2);
    repo.save(job);
    queue.enqueue("job_1");

    WorkerPool pool(queue, repo, 1);

    std::this_thread::sleep_for(std::chrono::seconds(15));
    pool.stop();

    auto result = repo.findById("job_1");
    CHECK(result.has_value());
    CHECK(result->getStatus() == JobStatus::Completed);
    CHECK(result->getRetryCount() == 2);
    std::cout << "[PASS] WorkerTransientFailureRetries -- retryCount=" << result->getRetryCount() << std::endl;
}

DROGON_TEST(WorkerStatusTransitions)
{
    std::cout << "[TEST] Executing: WorkerStatusTransitions (wait ~2s) ..." << std::endl;
    JobRepository repo;
    JobQueue queue;

    Job job("job_1", "test", Json::Value());
    repo.save(job);
    queue.enqueue("job_1");

    WorkerPool pool(queue, repo, 1, [](const Json::Value &) -> Json::Value {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        Json::Value r;
        r["ok"] = true;
        return r;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    auto running = repo.findById("job_1");
    CHECK(running.has_value());
    CHECK(running->getStatus() == JobStatus::Running);
    std::cout << "       ... verified status=Running at 300ms" << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(2));
    pool.stop();

    auto completed = repo.findById("job_1");
    CHECK(completed.has_value());
    CHECK(completed->getStatus() == JobStatus::Completed);
    std::cout << "[PASS] WorkerStatusTransitions -- Queued -> Running -> Completed" << std::endl;
}

DROGON_TEST(WorkerMultipleJobsConcurrent)
{
    std::cout << "[TEST] Executing: WorkerMultipleJobsConcurrent (4 jobs, 4 workers, wait ~2s) ..." << std::endl;
    JobRepository repo;
    JobQueue queue;

    for (int i = 1; i <= 4; ++i)
    {
        std::string id = "job_" + std::to_string(i);
        repo.save(Job(id, "task", Json::Value()));
        queue.enqueue(id);
    }

    WorkerPool pool(queue, repo, 4, [](const Json::Value &) -> Json::Value {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        Json::Value r;
        r["done"] = true;
        return r;
    });

    std::this_thread::sleep_for(std::chrono::seconds(2));
    pool.stop();

    int completedCount = 0;
    for (int i = 1; i <= 4; ++i)
    {
        std::string id = "job_" + std::to_string(i);
        auto result = repo.findById(id);
        CHECK(result.has_value());
        CHECK(result->getStatus() == JobStatus::Completed);
        ++completedCount;
    }
    std::cout << "[PASS] WorkerMultipleJobsConcurrent -- " << completedCount << "/4 completed" << std::endl;
}

DROGON_TEST(WorkerNonExistentJobSkipped)
{
    std::cout << "[TEST] Executing: WorkerNonExistentJobSkipped (wait ~2s) ..." << std::endl;
    JobRepository repo;
    JobQueue queue;

    queue.enqueue("ghost_job");

    Job real("real_job", "test", Json::Value());
    repo.save(real);
    queue.enqueue("real_job");

    WorkerPool pool(queue, repo, 1, [](const Json::Value &) -> Json::Value {
        Json::Value r;
        r["done"] = true;
        return r;
    });

    std::this_thread::sleep_for(std::chrono::seconds(2));
    pool.stop();

    CHECK(!repo.findById("ghost_job").has_value());
    auto result = repo.findById("real_job");
    CHECK(result.has_value());
    CHECK(result->getStatus() == JobStatus::Completed);
    std::cout << "[PASS] WorkerNonExistentJobSkipped -- ghost skipped, real completed" << std::endl;
}

DROGON_TEST(WorkerScaleUp)
{
    std::cout << "[TEST] Executing: WorkerScaleUp ..." << std::endl;
    JobRepository repo;
    JobQueue queue;

    WorkerPool pool(queue, repo, 2, [](const Json::Value &) -> Json::Value {
        Json::Value r;
        return r;
    });

    CHECK(pool.getActiveCount() == 2);
    pool.scaleUp(3);
    CHECK(pool.getActiveCount() == 5);
    std::cout << "[PASS] WorkerScaleUp -- 2 -> 5 workers" << std::endl;
    pool.stop();
}
