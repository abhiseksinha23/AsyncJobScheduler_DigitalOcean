#pragma once

#include <chrono>
#include <json/json.h>
#include <string>

enum class JobStatus
{
    Queued,
    Running,
    Completed,
    Failed,
    Retrying
};

inline std::string jobStatusToString(JobStatus status)
{
    switch (status)
    {
    case JobStatus::Queued:
        return "queued";
    case JobStatus::Running:
        return "running";
    case JobStatus::Completed:
        return "completed";
    case JobStatus::Failed:
        return "failed";
    case JobStatus::Retrying:
        return "retrying";
    }
    return "unknown";
}

class TransientError : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

class Job
{
public:
    Job() = default;

    Job(const std::string &id, const std::string &name, const Json::Value &payload)
        : id_(id), name_(name), payload_(payload),
          status_(JobStatus::Queued),
          createdAt_(std::chrono::system_clock::now()),
          updatedAt_(createdAt_) {}

    const std::string &getId() const { return id_; }
    void setId(const std::string &id) { id_ = id; }

    const std::string &getName() const { return name_; }
    void setName(const std::string &name) { name_ = name; }

    JobStatus getStatus() const { return status_; }
    void setStatus(JobStatus status) { status_ = status; }

    const Json::Value &getPayload() const { return payload_; }
    void setPayload(const Json::Value &payload) { payload_ = payload; }

    const Json::Value &getResult() const { return result_; }
    void setResult(const Json::Value &result) { result_ = result; }

    const std::string &getError() const { return error_; }
    void setError(const std::string &error) { error_ = error; }

    int getRetryCount() const { return retryCount_; }
    void setRetryCount(int count) { retryCount_ = count; }
    void incrementRetryCount() { ++retryCount_; }

    int getMaxRetries() const { return maxRetries_; }
    void setMaxRetries(int max) { maxRetries_ = max; }

    std::chrono::system_clock::time_point getCreatedAt() const { return createdAt_; }
    void setCreatedAt(std::chrono::system_clock::time_point t) { createdAt_ = t; }

    std::chrono::system_clock::time_point getUpdatedAt() const { return updatedAt_; }
    void setUpdatedAt(std::chrono::system_clock::time_point t) { updatedAt_ = t; }

    void markUpdated() { updatedAt_ = std::chrono::system_clock::now(); }

private:
    std::string id_;
    std::string name_;
    JobStatus status_{JobStatus::Queued};
    Json::Value payload_;
    Json::Value result_;
    std::string error_;
    int retryCount_{0};
    int maxRetries_{3};
    std::chrono::system_clock::time_point createdAt_;
    std::chrono::system_clock::time_point updatedAt_;
};
