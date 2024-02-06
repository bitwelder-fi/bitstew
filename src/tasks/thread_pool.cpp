/*
 * Copyright (C) 2023 bitWelder
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see
 * <http://www.gnu.org/licenses/>
 */

#include <meta/tasks/thread_pool.hpp>
#include <assert.hpp>
#include <utils/scope_value.hpp>

#include <atomic>
#include <deque>
#include <thread>
#include <mutex>

#include "../private/thread_pool.hpp"

namespace meta
{

class ThreadPool::Descriptor
{
    JobPtr popFrontJob()
    {
        auto job = jobs.front();
        jobs.pop_front();
        detail::JobPrivate::notifyJobScheduled(*job);
        return job;
    }

    JobPtr getNextJob()
    {
        UniqueLock lock(queueLock);
        auto condition = [this]()
        {
            // Bail out if there is a task to schedule, or stop thread is set.
            return stopSignalled || !jobs.empty();
        };
        lockCondition.wait(lock, condition);

        if (jobs.empty())
        {
            return {};
        }

        return popFrontJob();
    }

    void runSingleJob()
    {
        auto currentJob = getNextJob();
        if (!currentJob || stopSignalled || currentJob->isStopped())
        {
            return;
        }

        --idleThreadCount;
        {
            GuardLock lock(queueLock);
            scheduledJobs.push_back(currentJob);
        }
        detail::JobPrivate::runJob(*currentJob);

        {
            GuardLock lock(queueLock);
            std::erase(scheduledJobs, currentJob);
        }
        ++idleThreadCount;
    }

public:
    // The amount of threads to create.
    const std::size_t threadCount = 0u;
    // The number of idling threads.
    std::atomic_size_t idleThreadCount = 0u;
    // Locks the task queue.
    std::mutex queueLock;
    // Threads wait on new tasks.
    std::condition_variable lockCondition;
    // The scheduled jobs.
    std::deque<JobPtr> jobs;
    // The running jobs.
    std::deque<JobPtr> scheduledJobs;
    // The executor threads of the pool.
    std::vector<std::thread> threads;
    // Tells the thread pool to stop executing.
    std::atomic_bool stopSignalled = false;
    // Whether the pool is running.
    bool isRunning = false;

    explicit Descriptor(std::size_t threadCount) :
        threadCount(threadCount)
    {
    }

    void stopJobs()
    {
        GuardLock lock(queueLock);

        // Stop the queued jobs first.
        for (auto& job : jobs)
        {
            job->stop();
        }
        jobs.clear();

        // Then stop the scheduled jobs.
        for (auto& job : scheduledJobs)
        {
            job->stop();
        }
        scheduledJobs.clear();
    }

    static void threadMain(ThreadPool* self)
    {
        // Increase idle thread count before starting the thread loop.
        ++self->descriptor->idleThreadCount;

        while (!self->descriptor->stopSignalled)
        {
            self->descriptor->runSingleJob();
        }

        // Decrease idle thread count before exiting the thread loop.
        --self->descriptor->idleThreadCount;
    }
};


ThreadPool::ThreadPool(std::size_t threadCount) :
    descriptor(std::make_unique<ThreadPool::Descriptor>(threadCount))
{
}

ThreadPool::~ThreadPool()
{
    abortIfFail(!descriptor->isRunning);
}

void ThreadPool::start()
{
    abortIfFail(!descriptor->isRunning);
    descriptor->stopSignalled = false;
    descriptor->idleThreadCount = 0u;

    descriptor->threads.reserve(descriptor->threadCount);
    for (std::size_t i = 0u; i < descriptor->threadCount; ++i)
    {
        descriptor->threads.push_back(std::thread(&Descriptor::threadMain, this));
    }
    descriptor->isRunning = true;
    // Wait till the threads are all up and running.
    while (descriptor->idleThreadCount < descriptor->threadCount)
    {
        schedule(std::chrono::milliseconds(10));
    }
}

void ThreadPool::stop()
{
    abortIfFail(descriptor->isRunning);

    // Signal stop call.
    descriptor->stopSignalled = true;
    descriptor->stopJobs();

    // Notify all the threads to stop executing their jobs.
    descriptor->lockCondition.notify_all();
    schedule();

    // Wait till threads complete the tasks.
    while (descriptor->idleThreadCount > 0u)
    {
        schedule(std::chrono::milliseconds(10));
    }

    // Join the threads.
    for (auto& thread : descriptor->threads)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }

    descriptor->threads.clear();
    descriptor->isRunning = false;
}

bool ThreadPool::isBusy()
{
    UniqueLock lock(descriptor->queueLock);
    return !descriptor->jobs.empty() ||
           !descriptor->scheduledJobs.empty() ||
           descriptor->idleThreadCount < descriptor->threadCount;
}

bool ThreadPool::isRunning() const
{
    return descriptor->isRunning;
}

bool ThreadPool::isStopSignalled() const
{
    return descriptor->stopSignalled;
}

std::size_t ThreadPool::getThreadCount() const
{
    return descriptor->isRunning ? descriptor->threadCount : 0u;
}

std::size_t ThreadPool::getIdleCount() const
{
    return descriptor->idleThreadCount;
}

bool ThreadPool::pushJob(JobPtr job)
{
    if (descriptor->stopSignalled)
    {
        return false;
    }

    abortIfFail(job);

    const auto jobStatus = job->getStatus();
    abortIfFail(jobStatus == Job::Status::Deferred || jobStatus == Job::Status::Stopped);

    {
        GuardLock lock(descriptor->queueLock);
        descriptor->jobs.push_back(job);
        detail::JobPrivate::notifyJobQueued(*job);
    }

    descriptor->lockCondition.notify_one();

    return true;
}

std::size_t ThreadPool::pushMultipleJobs(std::vector<JobPtr> jobs)
{
    if (descriptor->stopSignalled)
    {
        return 0u;
    }

    abortIfFail(!jobs.empty());
    std::size_t result = 0u;

    {
        GuardLock lock(descriptor->queueLock);

        descriptor->jobs.insert(descriptor->jobs.end(), jobs.begin(), jobs.end());
        for (auto& job : jobs)
        {
            detail::JobPrivate::notifyJobQueued(*job);
            ++result;
        }
    }

    if (jobs.size() > 1)
    {
        descriptor->lockCondition.notify_all();
    }
    else
    {
        descriptor->lockCondition.notify_one();
    }

    return result;
}

std::size_t ThreadPool::getQueuedJobs() const
{
    return descriptor->jobs.size();
}

void ThreadPool::schedule()
{
    schedule(std::chrono::nanoseconds(1));
}

void ThreadPool::schedule(const std::chrono::nanoseconds& delay)
{
    std::this_thread::sleep_for(delay);
}


void async(JobPtr job)
{
    auto pool = Library::instance().threadPool();
    if (pool)
    {
        pool->pushJob(job);
    }
    else
    {
        detail::JobPrivate::setStatus(*job, Job::Status::Scheduled);
        detail::JobPrivate::runJob(*job);
    }
}

void yield()
{
    auto pool = Library::instance().threadPool();
    if (pool)
    {
        pool->schedule();
    }
}

} // namespace meta
