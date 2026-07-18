#include <drogon/drogon_test.h>
#include <iostream>
#include "../models/Job.h"

DROGON_TEST(JobDefaultConstructor)
{
    std::cout << "[TEST] Executing: JobDefaultConstructor ..." << std::endl;
    Job job;
    CHECK(job.getId().empty());
    CHECK(job.getName().empty());
    CHECK(job.getStatus() == JobStatus::Queued);
    CHECK(job.getRetryCount() == 0);
    CHECK(job.getMaxRetries() == 3);
    CHECK(job.getError().empty());
    std::cout << "[PASS] JobDefaultConstructor" << std::endl;
}

DROGON_TEST(JobParameterizedConstructor)
{
    std::cout << "[TEST] Executing: JobParameterizedConstructor ..." << std::endl;
    Json::Value payload;
    payload["key"] = "value";

    Job job("job_1", "test-job", payload);

    CHECK(job.getId() == "job_1");
    CHECK(job.getName() == "test-job");
    CHECK(job.getStatus() == JobStatus::Queued);
    CHECK(job.getPayload()["key"].asString() == "value");
    CHECK(job.getRetryCount() == 0);
    CHECK(job.getMaxRetries() == 3);

    auto now = std::chrono::system_clock::now();
    auto created = job.getCreatedAt();
    auto diff = std::chrono::duration_cast<std::chrono::seconds>(now - created).count();
    CHECK(diff < 2);
    std::cout << "[PASS] JobParameterizedConstructor" << std::endl;
}

DROGON_TEST(JobSetters)
{
    std::cout << "[TEST] Executing: JobSetters ..." << std::endl;
    Job job("job_1", "original", Json::Value());

    job.setName("renamed");
    CHECK(job.getName() == "renamed");

    job.setStatus(JobStatus::Running);
    CHECK(job.getStatus() == JobStatus::Running);

    job.setError("something went wrong");
    CHECK(job.getError() == "something went wrong");

    job.setRetryCount(2);
    CHECK(job.getRetryCount() == 2);

    job.setMaxRetries(5);
    CHECK(job.getMaxRetries() == 5);

    Json::Value result;
    result["data"] = "done";
    job.setResult(result);
    CHECK(job.getResult()["data"].asString() == "done");
    std::cout << "[PASS] JobSetters" << std::endl;
}

DROGON_TEST(JobIncrementRetryCount)
{
    std::cout << "[TEST] Executing: JobIncrementRetryCount ..." << std::endl;
    Job job("job_1", "test", Json::Value());

    CHECK(job.getRetryCount() == 0);
    job.incrementRetryCount();
    CHECK(job.getRetryCount() == 1);
    job.incrementRetryCount();
    CHECK(job.getRetryCount() == 2);
    std::cout << "[PASS] JobIncrementRetryCount" << std::endl;
}

DROGON_TEST(JobMarkUpdated)
{
    std::cout << "[TEST] Executing: JobMarkUpdated ..." << std::endl;
    Job job("job_1", "test", Json::Value());
    auto initial = job.getUpdatedAt();

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    job.markUpdated();

    CHECK(job.getUpdatedAt() > initial);
    std::cout << "[PASS] JobMarkUpdated" << std::endl;
}

DROGON_TEST(JobStatusToString)
{
    std::cout << "[TEST] Executing: JobStatusToString ..." << std::endl;
    CHECK(jobStatusToString(JobStatus::Queued) == "queued");
    CHECK(jobStatusToString(JobStatus::Running) == "running");
    CHECK(jobStatusToString(JobStatus::Completed) == "completed");
    CHECK(jobStatusToString(JobStatus::Failed) == "failed");
    CHECK(jobStatusToString(JobStatus::Retrying) == "retrying");
    std::cout << "[PASS] JobStatusToString" << std::endl;
}
