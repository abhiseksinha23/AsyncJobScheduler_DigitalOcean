#include <drogon/drogon_test.h>
#include <iostream>
#include <thread>
#include <vector>
#include "../repository/JobRepository.h"

DROGON_TEST(RepoSaveAndFind)
{
    std::cout << "[TEST] Executing: RepoSaveAndFind ..." << std::endl;
    JobRepository repo;
    Job job("job_1", "test", Json::Value());
    repo.save(job);

    auto found = repo.findById("job_1");
    CHECK(found.has_value());
    CHECK(found->getId() == "job_1");
    CHECK(found->getName() == "test");
    std::cout << "[PASS] RepoSaveAndFind" << std::endl;
}

DROGON_TEST(RepoFindNonExistent)
{
    std::cout << "[TEST] Executing: RepoFindNonExistent ..." << std::endl;
    JobRepository repo;
    auto found = repo.findById("does_not_exist");
    CHECK(!found.has_value());
    std::cout << "[PASS] RepoFindNonExistent" << std::endl;
}

DROGON_TEST(RepoFindAll)
{
    std::cout << "[TEST] Executing: RepoFindAll ..." << std::endl;
    JobRepository repo;
    repo.save(Job("job_1", "first", Json::Value()));
    repo.save(Job("job_2", "second", Json::Value()));
    repo.save(Job("job_3", "third", Json::Value()));

    auto all = repo.findAll();
    CHECK(all.size() == 3);
    std::cout << "[PASS] RepoFindAll -- found " << all.size() << " jobs" << std::endl;
}

DROGON_TEST(RepoFindAllEmpty)
{
    std::cout << "[TEST] Executing: RepoFindAllEmpty ..." << std::endl;
    JobRepository repo;
    auto all = repo.findAll();
    CHECK(all.size() == 0);
    std::cout << "[PASS] RepoFindAllEmpty" << std::endl;
}

DROGON_TEST(RepoRemove)
{
    std::cout << "[TEST] Executing: RepoRemove ..." << std::endl;
    JobRepository repo;
    repo.save(Job("job_1", "test", Json::Value()));

    bool removed = repo.remove("job_1");
    CHECK(removed == true);

    auto found = repo.findById("job_1");
    CHECK(!found.has_value());
    std::cout << "[PASS] RepoRemove" << std::endl;
}

DROGON_TEST(RepoRemoveNonExistent)
{
    std::cout << "[TEST] Executing: RepoRemoveNonExistent ..." << std::endl;
    JobRepository repo;
    bool removed = repo.remove("does_not_exist");
    CHECK(removed == false);
    std::cout << "[PASS] RepoRemoveNonExistent" << std::endl;
}

DROGON_TEST(RepoExists)
{
    std::cout << "[TEST] Executing: RepoExists ..." << std::endl;
    JobRepository repo;
    repo.save(Job("job_1", "test", Json::Value()));

    CHECK(repo.exists("job_1") == true);
    CHECK(repo.exists("job_2") == false);
    std::cout << "[PASS] RepoExists" << std::endl;
}

DROGON_TEST(RepoSaveOverwrites)
{
    std::cout << "[TEST] Executing: RepoSaveOverwrites ..." << std::endl;
    JobRepository repo;
    Job job("job_1", "original", Json::Value());
    repo.save(job);

    job.setName("updated");
    repo.save(job);

    auto found = repo.findById("job_1");
    CHECK(found->getName() == "updated");

    auto all = repo.findAll();
    CHECK(all.size() == 1);
    std::cout << "[PASS] RepoSaveOverwrites" << std::endl;
}

DROGON_TEST(RepoConcurrentSaves)
{
    std::cout << "[TEST] Executing: RepoConcurrentSaves (8 threads x 50 jobs) ..." << std::endl;
    JobRepository repo;
    const int numThreads = 8;
    const int jobsPerThread = 50;

    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; ++i)
    {
        threads.emplace_back([&repo, i, jobsPerThread]() {
            for (int j = 0; j < jobsPerThread; ++j)
            {
                std::string id = "t" + std::to_string(i) + "_j" + std::to_string(j);
                repo.save(Job(id, "job", Json::Value()));
            }
        });
    }

    for (auto &t : threads)
        t.join();

    auto all = repo.findAll();
    CHECK(all.size() == numThreads * jobsPerThread);
    std::cout << "[PASS] RepoConcurrentSaves -- stored " << all.size() << " jobs" << std::endl;
}
