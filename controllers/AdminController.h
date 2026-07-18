#pragma once

#include <drogon/HttpController.h>

#include "../workers/WorkerPool.h"

using namespace drogon;

class AdminController : public HttpController<AdminController>
{
public:
    METHOD_LIST_BEGIN

    ADD_METHOD_TO(AdminController::getWorkers, "/admin/workers", Get);
    ADD_METHOD_TO(AdminController::setWorkers, "/admin/workers", Post);

    METHOD_LIST_END

    void getWorkers(
        const HttpRequestPtr &req,
        std::function<void(const HttpResponsePtr &)> &&callback);

    void setWorkers(
        const HttpRequestPtr &req,
        std::function<void(const HttpResponsePtr &)> &&callback);

    static void setPool(WorkerPool *pool) { pool_ = pool; }

private:
    static WorkerPool *pool_;
};
