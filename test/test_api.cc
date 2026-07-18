#include <drogon/drogon_test.h>
#include <drogon/HttpClient.h>
#include <drogon/drogon.h>
#include <iostream>

using namespace drogon;

static HttpClientPtr getClient()
{
    return HttpClient::newHttpClient("http://127.0.0.1:8080");
}

DROGON_TEST(ApiCreateJobSuccess)
{
    std::cout << "[TEST] Executing: ApiCreateJobSuccess ..." << std::endl;
    auto client = getClient();
    Json::Value body;
    body["name"] = "test-job";
    body["payload"]["input"] = "data";

    auto req = HttpRequest::newHttpJsonRequest(body);
    req->setMethod(Post);
    req->setPath("/jobs");

    auto [result, resp] = client->sendRequest(req, 5.0);
    CHECK(result == ReqResult::Ok);
    CHECK(resp->getStatusCode() == k202Accepted);

    auto json = resp->getJsonObject();
    CHECK(json != nullptr);
    CHECK((*json)["status"].asString() == "queued");
    CHECK(!(*json)["id"].asString().empty());
    CHECK((*json)["name"].asString() == "test-job");
    std::cout << "[PASS] ApiCreateJobSuccess -- 202 Accepted, id=" << (*json)["id"].asString() << std::endl;
}

DROGON_TEST(ApiCreateJobMissingName)
{
    std::cout << "[TEST] Executing: ApiCreateJobMissingName ..." << std::endl;
    auto client = getClient();
    Json::Value body;
    body["payload"]["input"] = "data";

    auto req = HttpRequest::newHttpJsonRequest(body);
    req->setMethod(Post);
    req->setPath("/jobs");

    auto [result, resp] = client->sendRequest(req, 5.0);
    CHECK(result == ReqResult::Ok);
    CHECK(resp->getStatusCode() == k400BadRequest);
    std::cout << "[PASS] ApiCreateJobMissingName -- 400 Bad Request" << std::endl;
}

DROGON_TEST(ApiGetAllJobsEmpty)
{
    std::cout << "[TEST] Executing: ApiGetAllJobsEmpty ..." << std::endl;
    auto client = getClient();
    auto req = HttpRequest::newHttpRequest();
    req->setMethod(Get);
    req->setPath("/jobs");

    auto [result, resp] = client->sendRequest(req, 5.0);
    CHECK(result == ReqResult::Ok);
    CHECK(resp->getStatusCode() == k200OK);
    std::cout << "[PASS] ApiGetAllJobsEmpty -- 200 OK" << std::endl;
}

DROGON_TEST(ApiGetJobNotFound)
{
    std::cout << "[TEST] Executing: ApiGetJobNotFound ..." << std::endl;
    auto client = getClient();
    auto req = HttpRequest::newHttpRequest();
    req->setMethod(Get);
    req->setPath("/jobs/nonexistent_id");

    auto [result, resp] = client->sendRequest(req, 5.0);
    CHECK(result == ReqResult::Ok);
    CHECK(resp->getStatusCode() == k404NotFound);
    std::cout << "[PASS] ApiGetJobNotFound -- 404 Not Found" << std::endl;
}

DROGON_TEST(ApiGetJobStatusNotFound)
{
    std::cout << "[TEST] Executing: ApiGetJobStatusNotFound ..." << std::endl;
    auto client = getClient();
    auto req = HttpRequest::newHttpRequest();
    req->setMethod(Get);
    req->setPath("/jobs/nonexistent_id/status");

    auto [result, resp] = client->sendRequest(req, 5.0);
    CHECK(result == ReqResult::Ok);
    CHECK(resp->getStatusCode() == k404NotFound);
    std::cout << "[PASS] ApiGetJobStatusNotFound -- 404 Not Found" << std::endl;
}

DROGON_TEST(ApiUpdateJobNotFound)
{
    std::cout << "[TEST] Executing: ApiUpdateJobNotFound ..." << std::endl;
    auto client = getClient();
    Json::Value body;
    body["name"] = "new-name";

    auto req = HttpRequest::newHttpJsonRequest(body);
    req->setMethod(Put);
    req->setPath("/jobs/nonexistent_id");

    auto [result, resp] = client->sendRequest(req, 5.0);
    CHECK(result == ReqResult::Ok);
    CHECK(resp->getStatusCode() == k404NotFound);
    std::cout << "[PASS] ApiUpdateJobNotFound -- 404 Not Found" << std::endl;
}

DROGON_TEST(ApiUpdateJobMissingName)
{
    std::cout << "[TEST] Executing: ApiUpdateJobMissingName ..." << std::endl;
    auto client = getClient();
    Json::Value body;
    body["other"] = "field";

    auto req = HttpRequest::newHttpJsonRequest(body);
    req->setMethod(Put);
    req->setPath("/jobs/some_id");

    auto [result, resp] = client->sendRequest(req, 5.0);
    CHECK(result == ReqResult::Ok);
    CHECK(resp->getStatusCode() == k400BadRequest);
    std::cout << "[PASS] ApiUpdateJobMissingName -- 400 Bad Request" << std::endl;
}

DROGON_TEST(ApiDeleteJobNotFound)
{
    std::cout << "[TEST] Executing: ApiDeleteJobNotFound ..." << std::endl;
    auto client = getClient();
    auto req = HttpRequest::newHttpRequest();
    req->setMethod(Delete);
    req->setPath("/jobs/nonexistent_id");

    auto [result, resp] = client->sendRequest(req, 5.0);
    CHECK(result == ReqResult::Ok);
    CHECK(resp->getStatusCode() == k404NotFound);
    std::cout << "[PASS] ApiDeleteJobNotFound -- 404 Not Found" << std::endl;
}

DROGON_TEST(ApiGetWorkers)
{
    std::cout << "[TEST] Executing: ApiGetWorkers ..." << std::endl;
    auto client = getClient();
    auto req = HttpRequest::newHttpRequest();
    req->setMethod(Get);
    req->setPath("/admin/workers");

    auto [result, resp] = client->sendRequest(req, 5.0);
    CHECK(result == ReqResult::Ok);
    CHECK(resp->getStatusCode() == k200OK);

    auto json = resp->getJsonObject();
    CHECK(json != nullptr);
    CHECK((*json)["activeWorkers"].asInt() > 0);
    std::cout << "[PASS] ApiGetWorkers -- activeWorkers=" << (*json)["activeWorkers"].asInt() << std::endl;
}

DROGON_TEST(ApiSetWorkersInvalidZero)
{
    std::cout << "[TEST] Executing: ApiSetWorkersInvalidZero ..." << std::endl;
    auto client = getClient();
    Json::Value body;
    body["count"] = 0;

    auto req = HttpRequest::newHttpJsonRequest(body);
    req->setMethod(Post);
    req->setPath("/admin/workers");

    auto [result, resp] = client->sendRequest(req, 5.0);
    CHECK(result == ReqResult::Ok);
    CHECK(resp->getStatusCode() == k400BadRequest);
    std::cout << "[PASS] ApiSetWorkersInvalidZero -- 400 Bad Request" << std::endl;
}

DROGON_TEST(ApiSetWorkersMissingCount)
{
    std::cout << "[TEST] Executing: ApiSetWorkersMissingCount ..." << std::endl;
    auto client = getClient();
    Json::Value body;
    body["other"] = 5;

    auto req = HttpRequest::newHttpJsonRequest(body);
    req->setMethod(Post);
    req->setPath("/admin/workers");

    auto [result, resp] = client->sendRequest(req, 5.0);
    CHECK(result == ReqResult::Ok);
    CHECK(resp->getStatusCode() == k400BadRequest);
    std::cout << "[PASS] ApiSetWorkersMissingCount -- 400 Bad Request" << std::endl;
}

DROGON_TEST(ApiJobLifecycle)
{
    std::cout << "[TEST] Executing: ApiJobLifecycle (create -> wait 2s -> verify completed) ..." << std::endl;
    auto client = getClient();

    Json::Value body;
    body["name"] = "lifecycle-test";
    body["payload"]["data"] = "test";

    auto req = HttpRequest::newHttpJsonRequest(body);
    req->setMethod(Post);
    req->setPath("/jobs");

    auto [r1, resp1] = client->sendRequest(req, 5.0);
    CHECK(r1 == ReqResult::Ok);
    CHECK(resp1->getStatusCode() == k202Accepted);

    auto json1 = resp1->getJsonObject();
    std::string jobId = (*json1)["id"].asString();
    std::cout << "       ... created job id=" << jobId << ", waiting 2s for processing" << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(2));

    auto statusReq = HttpRequest::newHttpRequest();
    statusReq->setMethod(Get);
    statusReq->setPath("/jobs/" + jobId + "/status");

    auto [r2, resp2] = client->sendRequest(statusReq, 5.0);
    CHECK(r2 == ReqResult::Ok);
    auto json2 = resp2->getJsonObject();
    CHECK(json2 != nullptr);
    CHECK((*json2)["status"].asString() == "completed");
    CHECK((*json2)["retryCount"].asInt() == 0);
    std::cout << "[PASS] ApiJobLifecycle -- job completed successfully" << std::endl;
}

DROGON_TEST(ApiConflictOnRunningJob)
{
    std::cout << "[TEST] Executing: ApiConflictOnRunningJob (create -> attempt PUT/DELETE while running) ..." << std::endl;
    auto client = getClient();

    Json::Value body;
    body["name"] = "conflict-test";
    body["payload"]["simulate_failure"] = true;

    auto req = HttpRequest::newHttpJsonRequest(body);
    req->setMethod(Post);
    req->setPath("/jobs");

    auto [r1, resp1] = client->sendRequest(req, 5.0);
    CHECK(r1 == ReqResult::Ok);
    auto json1 = resp1->getJsonObject();
    std::string jobId = (*json1)["id"].asString();
    std::cout << "       ... created job id=" << jobId << ", waiting 500ms for it to start running" << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    Json::Value updateBody;
    updateBody["name"] = "renamed";
    auto updateReq = HttpRequest::newHttpJsonRequest(updateBody);
    updateReq->setMethod(Put);
    updateReq->setPath("/jobs/" + jobId);

    auto [r2, resp2] = client->sendRequest(updateReq, 5.0);
    CHECK(r2 == ReqResult::Ok);
    CHECK(resp2->getStatusCode() == k409Conflict);
    std::cout << "       ... PUT returned 409 Conflict (correct)" << std::endl;

    auto delReq = HttpRequest::newHttpRequest();
    delReq->setMethod(Delete);
    delReq->setPath("/jobs/" + jobId);

    auto [r3, resp3] = client->sendRequest(delReq, 5.0);
    CHECK(r3 == ReqResult::Ok);
    CHECK(resp3->getStatusCode() == k409Conflict);
    std::cout << "[PASS] ApiConflictOnRunningJob -- both PUT and DELETE returned 409" << std::endl;
}
