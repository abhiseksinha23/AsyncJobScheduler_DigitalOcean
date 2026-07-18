#include "AdminController.h"

WorkerPool *AdminController::pool_ = nullptr;

void AdminController::getWorkers(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback)
{
    Json::Value response;
    response["activeWorkers"] = pool_->getActiveCount();

    callback(HttpResponse::newHttpJsonResponse(response));
}

void AdminController::setWorkers(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback)
{
    auto json = req->getJsonObject();

    if (!json || !json->isMember("count"))
    {
        auto resp = HttpResponse::newHttpResponse();
        resp->setStatusCode(k400BadRequest);
        resp->setContentTypeCode(CT_APPLICATION_JSON);
        Json::Value err;
        err["error"] = "Missing required field: count";
        resp->setBody(err.toStyledString());
        callback(resp);
        return;
    }

    int desired = (*json)["count"].asInt();
    if (desired < 1)
    {
        auto resp = HttpResponse::newHttpResponse();
        resp->setStatusCode(k400BadRequest);
        resp->setContentTypeCode(CT_APPLICATION_JSON);
        Json::Value err;
        err["error"] = "Worker count must be at least 1";
        resp->setBody(err.toStyledString());
        callback(resp);
        return;
    }

    int current = pool_->getActiveCount();
    int delta = desired - current;

    if (delta > 0)
        pool_->scaleUp(static_cast<size_t>(delta));
    else if (delta < 0)
        pool_->scaleDown(static_cast<size_t>(-delta));

    Json::Value response;
    response["previousWorkers"] = current;
    response["requestedWorkers"] = desired;
    response["activeWorkers"] = pool_->getActiveCount();

    callback(HttpResponse::newHttpJsonResponse(response));
}
