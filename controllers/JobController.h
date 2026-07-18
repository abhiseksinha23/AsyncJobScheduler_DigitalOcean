#pragma once

#include <drogon/HttpController.h>

#include "../repository/JobRepository.h"
#include "../services/JobService.h"
#include "../workers/JobQueue.h"

using namespace drogon;

class JobController : public HttpController<JobController>
{
public:
    METHOD_LIST_BEGIN

    ADD_METHOD_TO(JobController::createJob, "/jobs", Post);
    ADD_METHOD_TO(JobController::createJob, "/jobs/", Post);
    ADD_METHOD_TO(JobController::getAllJobs, "/jobs", Get);
    ADD_METHOD_TO(JobController::getAllJobs, "/jobs/", Get);
    ADD_METHOD_TO(JobController::getJob, "/jobs/{1}", Get);
    ADD_METHOD_TO(JobController::getJobStatus, "/jobs/{1}/status", Get);
    ADD_METHOD_TO(JobController::updateJob, "/jobs/{1}", Put);
    ADD_METHOD_TO(JobController::deleteJob, "/jobs/{1}", Delete);

    METHOD_LIST_END

    void createJob(
        const HttpRequestPtr &req,
        std::function<void(const HttpResponsePtr &)> &&callback);

    void getAllJobs(
        const HttpRequestPtr &req,
        std::function<void(const HttpResponsePtr &)> &&callback);

    void getJob(
        const HttpRequestPtr &req,
        std::function<void(const HttpResponsePtr &)> &&callback,
        std::string id);

    void getJobStatus(
        const HttpRequestPtr &req,
        std::function<void(const HttpResponsePtr &)> &&callback,
        std::string id);

    void updateJob(
        const HttpRequestPtr &req,
        std::function<void(const HttpResponsePtr &)> &&callback,
        std::string id);

    void deleteJob(
        const HttpRequestPtr &req,
        std::function<void(const HttpResponsePtr &)> &&callback,
        std::string id);

    static void setService(JobService *service) { service_ = service; }

private:
    static JobService *service_;
};
