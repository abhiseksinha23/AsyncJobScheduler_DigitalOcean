cmake_minimum_required(VERSION 3.10)
project(AsyncJobScheduler_test CXX)

set(TEST_SOURCES
    test_main.cc
    test_job.cc
    test_queue.cc
    test_repository.cc
    test_worker.cc
    test_api.cc
)

set(APP_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../controllers/JobController.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/../controllers/AdminController.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/../controllers/rootController.cc
)

add_executable(${PROJECT_NAME} ${TEST_SOURCES} ${APP_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)

target_link_libraries(${PROJECT_NAME} PRIVATE Drogon::Drogon)

ParseAndAddDrogonTests(${PROJECT_NAME})
