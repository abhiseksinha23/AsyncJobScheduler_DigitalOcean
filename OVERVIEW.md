# Project Overview

> A quick guide for anyone landing on this project for the first time.  
> Read time: ~3 minutes.

---

## What Is This?

AsyncJobScheduler is a **REST API service** that processes tasks in the background. You send it a job via HTTP, get an ID back instantly, and the system processes it asynchronously using worker threads. You can check the status anytime.

Think of it like a simplified version of Celery (Python) or Sidekiq (Ruby), but built from scratch in C++20.

---

## Why Does It Exist?

This project demonstrates how to build an async job processing system from the ground up, covering:

- REST API design with proper HTTP status codes
- Thread-safe concurrent programming (mutexes, condition variables, atomics)
- Producer-consumer architecture
- Dynamic runtime scaling
- Retry logic with exponential backoff
- Concurrency control to prevent race conditions
- Comprehensive testing (unit + integration)
- CI/CD automation

---

## How It Works (30-Second Version)

```
Client sends POST /jobs  -->  Server saves job, returns ID immediately (202)
                                    |
                                    v
                              Job enters Queue
                                    |
                                    v
                          Worker thread picks it up
                                    |
                                    v
                          Processes it (success/fail)
                                    |
                                    v
                    Client polls GET /jobs/{id}/status  -->  Gets result
```

---

## Key Concepts

| Concept | What It Means Here |
|---------|-------------------|
| **Job** | A unit of work with a name, payload, status, and result |
| **Queue** | A thread-safe FIFO buffer between the API and workers |
| **Worker Pool** | Background threads that pick up and process jobs |
| **Repository** | In-memory storage (like a database, but in RAM) |
| **Service** | Business logic layer that coordinates everything |
| **Controller** | HTTP endpoint handler (translates HTTP to service calls) |

---

## The Flow of Data

1. **Client** sends `POST /jobs` with a name and payload
2. **JobController** receives it, asks **JobService** to create the job
3. **JobService** saves the job to **JobRepository** and puts the ID into **JobQueue**
4. **JobController** responds `202 Accepted` with the job ID (client is free)
5. A **WorkerPool** thread calls `dequeue()` on **JobQueue** (blocks until a job arrives)
6. Worker loads the job from **JobRepository**, sets status to `running`
7. Worker executes the processing logic (configurable, default simulates 10s work)
8. On success: status becomes `completed`, result is saved
9. On failure: either retries (transient) or status becomes `failed`
10. **Client** polls `GET /jobs/{id}/status` at any time to check progress

---

## Tech Stack

| Layer | Technology |
|-------|-----------|
| Language | C++20 |
| Web Framework | [Drogon](https://github.com/drogonframework/drogon) |
| Build System | CMake |
| Storage | In-memory (`std::unordered_map`) |
| Concurrency | `std::thread`, `std::mutex`, `std::condition_variable`, `std::atomic` |
| Testing | Drogon Test Framework (`drogon_test`) |
| CI/CD | GitHub Actions |

---

## API at a Glance

| Action | Command |
|--------|---------|
| Submit a job | `POST /jobs` with `{"name": "...", "payload": {...}}` |
| Check status | `GET /jobs/{id}/status` |
| Get full details | `GET /jobs/{id}` |
| List all jobs | `GET /jobs` |
| Update a job | `PUT /jobs/{id}` with `{"name": "..."}` |
| Delete a job | `DELETE /jobs/{id}` |
| View worker count | `GET /admin/workers` |
| Scale workers | `POST /admin/workers` with `{"count": N}` |

---

## Job States

```
 queued  -->  running  -->  completed
                |
                +--> retrying (transient error, will retry)
                |
                +--> failed (permanent error or retries exhausted)
```

---

## Project Layout (Simplified)

```
AsyncJobScheduler/
├── main.cc              # Wires everything together
├── models/Job.h         # The Job data class
├── repository/          # Where jobs are stored
├── services/            # Business logic
├── workers/             # Queue + thread pool
├── controllers/         # HTTP endpoints
├── test/                # 41 automated tests
├── .github/workflows/   # CI pipeline
├── DOCUMENTATION.md     # Full detailed documentation
└── README.md            # Project reference
```

---

## Quick Start (4 Commands)

```bash
# 1. Build
mkdir -p build && cd build && cmake .. && cmake --build .

# 2. Run
./AsyncJobScheduler

# 3. Test it
curl -X POST http://localhost:8080/jobs \
  -H "Content-Type: application/json" \
  -d '{"name": "hello", "payload": {"data": "world"}}'

# 4. Check result (after ~10s)
curl http://localhost:8080/jobs/job_1/status
```

---

## What Makes This Non-Trivial

This isn't just a CRUD app. Here's what's interesting under the hood:

1. **True async** -- the API responds before work is done (202, not 200)
2. **Thread pool with dynamic scaling** -- add/remove workers at runtime without restarting
3. **Graceful shutdown** -- workers finish current jobs before exiting
4. **Retry with backoff** -- transient failures retry with 2^n second delays
5. **Concurrency guards** -- can't modify a job while a worker is processing it (409 Conflict)
6. **Pluggable processing** -- swap in any logic via `std::function`, no code changes to the pool
7. **Full test coverage** -- 41 tests including timing-based assertions on async behavior

---

## Where to Go Next

| Want to... | Read |
|-----------|------|
| Set up and run the project | [DOCUMENTATION.md, Section 6](DOCUMENTATION.md#6-getting-started) |
| Understand the architecture | [DOCUMENTATION.md, Section 2](DOCUMENTATION.md#2-architecture) |
| See the full API reference | [DOCUMENTATION.md, Section 7](DOCUMENTATION.md#7-api-reference) |
| Understand retry logic | [DOCUMENTATION.md, Section 9](DOCUMENTATION.md#9-retry-logic) |
| Run or write tests | [DOCUMENTATION.md, Section 11](DOCUMENTATION.md#11-testing) |
| See known limitations | [DOCUMENTATION.md, Section 13](DOCUMENTATION.md#13-known-limitations) |
| View design patterns used | [DOCUMENTATION.md, Section 3](DOCUMENTATION.md#3-design-patterns--principles) |

---

*For the complete reference, see [DOCUMENTATION.md](DOCUMENTATION.md).*
