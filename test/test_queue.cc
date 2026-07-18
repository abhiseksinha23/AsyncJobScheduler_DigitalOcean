#include <drogon/drogon_test.h>
#include <iostream>
#include <thread>
#include <chrono>
#include "../workers/JobQueue.h"

DROGON_TEST(QueueEnqueueDequeue)
{
    std::cout << "[TEST] Executing: QueueEnqueueDequeue ..." << std::endl;
    JobQueue queue;
    queue.enqueue("job_1");

    auto result = queue.dequeue();
    CHECK(result.has_value());
    CHECK(*result == "job_1");
    std::cout << "[PASS] QueueEnqueueDequeue" << std::endl;
}

DROGON_TEST(QueueFIFOOrder)
{
    std::cout << "[TEST] Executing: QueueFIFOOrder ..." << std::endl;
    JobQueue queue;
    queue.enqueue("job_1");
    queue.enqueue("job_2");
    queue.enqueue("job_3");

    CHECK(*queue.dequeue() == "job_1");
    CHECK(*queue.dequeue() == "job_2");
    CHECK(*queue.dequeue() == "job_3");
    std::cout << "[PASS] QueueFIFOOrder" << std::endl;
}

DROGON_TEST(QueueBlocksUntilEnqueue)
{
    std::cout << "[TEST] Executing: QueueBlocksUntilEnqueue ..." << std::endl;
    JobQueue queue;
    std::string received;

    std::thread consumer([&]() {
        auto result = queue.dequeue();
        if (result)
            received = *result;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    queue.enqueue("delayed_job");
    consumer.join();

    CHECK(received == "delayed_job");
    std::cout << "[PASS] QueueBlocksUntilEnqueue" << std::endl;
}

DROGON_TEST(QueueShutdownUnblocksDequeue)
{
    std::cout << "[TEST] Executing: QueueShutdownUnblocksDequeue ..." << std::endl;
    JobQueue queue;
    std::optional<std::string> result;

    std::thread consumer([&]() {
        result = queue.dequeue();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    queue.shutdown();
    consumer.join();

    CHECK(!result.has_value());
    std::cout << "[PASS] QueueShutdownUnblocksDequeue" << std::endl;
}

DROGON_TEST(QueueDrainAfterShutdown)
{
    std::cout << "[TEST] Executing: QueueDrainAfterShutdown ..." << std::endl;
    JobQueue queue;
    queue.enqueue("job_1");
    queue.enqueue("job_2");
    queue.shutdown();

    auto r1 = queue.dequeue();
    auto r2 = queue.dequeue();
    auto r3 = queue.dequeue();

    CHECK(r1.has_value());
    CHECK(*r1 == "job_1");
    CHECK(r2.has_value());
    CHECK(*r2 == "job_2");
    CHECK(!r3.has_value());
    std::cout << "[PASS] QueueDrainAfterShutdown" << std::endl;
}

DROGON_TEST(QueueMultipleProducers)
{
    std::cout << "[TEST] Executing: QueueMultipleProducers (4 producers x 10 jobs) ..." << std::endl;
    JobQueue queue;
    const int numProducers = 4;
    const int jobsPerProducer = 10;

    std::vector<std::thread> producers;
    for (int i = 0; i < numProducers; ++i)
    {
        producers.emplace_back([&queue, i, jobsPerProducer]() {
            for (int j = 0; j < jobsPerProducer; ++j)
                queue.enqueue("p" + std::to_string(i) + "_j" + std::to_string(j));
        });
    }

    for (auto &t : producers)
        t.join();

    int count = 0;
    queue.shutdown();
    while (auto r = queue.dequeue())
        ++count;

    CHECK(count == numProducers * jobsPerProducer);
    std::cout << "[PASS] QueueMultipleProducers -- dequeued " << count << " items" << std::endl;
}
