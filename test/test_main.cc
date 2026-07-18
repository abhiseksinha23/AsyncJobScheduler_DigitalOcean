#define DROGON_TEST_MAIN
#include <drogon/drogon_test.h>
#include <drogon/drogon.h>
#include <iostream>
#include <chrono>

#include "../controllers/JobController.h"
#include "../controllers/AdminController.h"
#include "../repository/JobRepository.h"
#include "../services/JobService.h"
#include "../workers/JobQueue.h"
#include "../workers/WorkerPool.h"

static JobRepository repository;
static JobQueue queue;
static JobService service(repository, queue);
static std::unique_ptr<WorkerPool> pool;

int main(int argc, char **argv)
{
    using namespace drogon;

    std::cout << "\n========================================" << std::endl;
    std::cout << "  AsyncJobScheduler Test Suite" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "[SETUP] Initializing test environment..." << std::endl;

    auto startTime = std::chrono::steady_clock::now();

    pool = std::make_unique<WorkerPool>(queue, repository, 2,
        [](const Json::Value &payload) -> Json::Value {
            bool simulateFailure = payload.isMember("simulate_failure") &&
                                   payload["simulate_failure"].asBool();
            std::this_thread::sleep_for(std::chrono::seconds(1));
            if (simulateFailure)
                throw TransientError("Simulated transient error");
            Json::Value r;
            r["message"] = "Job processed successfully";
            return r;
        });

    JobController::setService(&service);
    AdminController::setPool(pool.get());

    std::cout << "[SETUP] Worker pool created (2 workers, 1s processing time)" << std::endl;
    std::cout << "[SETUP] Starting Drogon HTTP server on 127.0.0.1:8080..." << std::endl;

    std::promise<void> p1;
    std::future<void> f1 = p1.get_future();

    std::thread thr([&]() {
        app().addListener("127.0.0.1", 8080);
        app().setThreadNum(2);
        app().getLoop()->queueInLoop([&p1]() { p1.set_value(); });
        app().run();
    });

    f1.get();
    std::cout << "[SETUP] Server ready. Running tests...\n" << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    int status = test::run(argc, argv);

    std::cout << "----------------------------------------" << std::endl;

    auto endTime = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

    std::cout << "\n[TEARDOWN] Stopping server and worker pool..." << std::endl;

    app().getLoop()->queueInLoop([]() { app().quit(); });
    thr.join();
    pool->stop();

    std::cout << "[TEARDOWN] Cleanup complete." << std::endl;
    std::cout << "\n========================================" << std::endl;
    std::cout << "  Total time: " << elapsed / 1000.0 << "s" << std::endl;
    std::cout << "  Result: " << (status == 0 ? "ALL TESTS PASSED" : "SOME TESTS FAILED") << std::endl;
    std::cout << "========================================\n" << std::endl;

    return status;
}
