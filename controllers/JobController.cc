#include "JobController.h"

JobService *JobController::service_ = nullptr;

void JobController::createJob(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback)
{
    auto json = req->getJsonObject();

    if (!json || !json->isMember("name"))
    {
        auto resp = HttpResponse::newHttpResponse();
        resp->setStatusCode(k400BadRequest);
        resp->setContentTypeCode(CT_APPLICATION_JSON);
        Json::Value err;
        err["error"] = "Missing required field: name";
        resp->setBody(err.toStyledString());
        callback(resp);
        return;
    }

    Json::Value payload = json->get("payload", Json::Value(Json::objectValue));
    auto job = service_->createJob((*json)["name"].asString(), payload);

    Json::Value response;
    response["id"] = job.getId();
    response["name"] = job.getName();
    response["status"] = jobStatusToString(job.getStatus());

    auto resp = HttpResponse::newHttpJsonResponse(response);
    resp->setStatusCode(k202Accepted);
    callback(resp);
}

void JobController::getAllJobs(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback)
{
    auto jobs = service_->getAllJobs();

    Json::Value response(Json::arrayValue);
    for (const auto &job : jobs)
    {
        Json::Value j;
        j["id"] = job.getId();
        j["name"] = job.getName();
        j["status"] = jobStatusToString(job.getStatus());
        response.append(j);
    }

    callback(HttpResponse::newHttpJsonResponse(response));
}

void JobController::getJob(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback,
    std::string id)
{
    auto job = service_->getJob(id);

    if (!job)
    {
        auto resp = HttpResponse::newHttpResponse();
        resp->setStatusCode(k404NotFound);
        callback(resp);
        return;
    }

    Json::Value response;
    response["id"] = job->getId();
    response["name"] = job->getName();
    response["status"] = jobStatusToString(job->getStatus());
    response["payload"] = job->getPayload();
    response["result"] = job->getResult();
    response["retryCount"] = job->getRetryCount();
    response["maxRetries"] = job->getMaxRetries();
    if (!job->getError().empty())
        response["error"] = job->getError();

    callback(HttpResponse::newHttpJsonResponse(response));
}

void JobController::getJobStatus(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback,
    std::string id)
{
    auto job = service_->getJob(id);

    if (!job)
    {
        auto resp = HttpResponse::newHttpResponse();
        resp->setStatusCode(k404NotFound);
        callback(resp);
        return;
    }

    Json::Value response;
    response["id"] = job->getId();
    response["status"] = jobStatusToString(job->getStatus());
    response["retryCount"] = job->getRetryCount();
    response["maxRetries"] = job->getMaxRetries();

    if (job->getStatus() == JobStatus::Completed)
        response["result"] = job->getResult();
    else if (job->getStatus() == JobStatus::Failed || job->getStatus() == JobStatus::Retrying)
        response["error"] = job->getError();

    callback(HttpResponse::newHttpJsonResponse(response));
}

void JobController::updateJob(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback,
    std::string id)
{
    auto json = req->getJsonObject();

    if (!json || !json->isMember("name"))
    {
        auto resp = HttpResponse::newHttpResponse();
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
    }

    auto existing = service_->getJob(id);
    if (!existing)
    {
        auto resp = HttpResponse::newHttpResponse();
        resp->setStatusCode(k404NotFound);
        callback(resp);
        return;
    }

    if (existing->getStatus() == JobStatus::Running || existing->getStatus() == JobStatus::Retrying)
    {
        auto resp = HttpResponse::newHttpResponse();
        resp->setStatusCode(k409Conflict);
        resp->setContentTypeCode(CT_APPLICATION_JSON);
        Json::Value err;
        err["error"] = "Cannot modify a job that is currently " + jobStatusToString(existing->getStatus());
        resp->setBody(err.toStyledString());
        callback(resp);
        return;
    }

    auto job = service_->updateJob(id, (*json)["name"].asString());

    Json::Value response;
    response["id"] = job->getId();
    response["name"] = job->getName();
    response["status"] = jobStatusToString(job->getStatus());

    callback(HttpResponse::newHttpJsonResponse(response));
}

void JobController::deleteJob(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback,
    std::string id)
{
    auto existing = service_->getJob(id);
    if (!existing)
    {
        auto resp = HttpResponse::newHttpResponse();
        resp->setStatusCode(k404NotFound);
        callback(resp);
        return;
    }

    if (existing->getStatus() == JobStatus::Running || existing->getStatus() == JobStatus::Retrying)
    {
        auto resp = HttpResponse::newHttpResponse();
        resp->setStatusCode(k409Conflict);
        resp->setContentTypeCode(CT_APPLICATION_JSON);
        Json::Value err;
        err["error"] = "Cannot delete a job that is currently " + jobStatusToString(existing->getStatus());
        resp->setBody(err.toStyledString());
        callback(resp);
        return;
    }

    service_->deleteJob(id);

    Json::Value response;
    response["message"] = "Job deleted";
    response["jobId"] = id;

    callback(HttpResponse::newHttpJsonResponse(response));
}
