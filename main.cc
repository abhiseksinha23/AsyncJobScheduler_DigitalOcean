#include <cstdlib>
#include <drogon/drogon.h>
#include <iostream>

#include "controllers/AdminController.h"
#include "controllers/JobController.h"
#include "repository/JobRepository.h"
#include "services/JobService.h"
#include "workers/JobQueue.h"
#include "workers/WorkerPool.h"

int main()
{
    int workerCount = 4;
    const char *envVal = std::getenv("WORKER_COUNT");
    if (envVal)
    {
        int parsed = std::atoi(envVal);
        if (parsed > 0)
            workerCount = parsed;
    }

    JobRepository repository;
    JobQueue queue;
    JobService service(repository, queue);
    WorkerPool pool(queue, repository, static_cast<size_t>(workerCount));

    JobController::setService(&service);
    AdminController::setPool(&pool);

    std::cout << "Starting AsyncJobScheduler on port 8080" << std::endl;
    std::cout << "Worker threads: " << workerCount << std::endl;

    drogon::app().addListener("0.0.0.0", 8080);
    drogon::app().setThreadNum(4);
    drogon::app().run();

    pool.stop();

    return 0;
}
