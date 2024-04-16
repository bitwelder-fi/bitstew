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

#include <meta/tasks/job.hpp>
#include <meta/tasks/thread_pool.hpp>
#include <assert.hpp>
#include <meta/utility/scope_value.hpp>

#include <atomic>
#include <condition_variable>
#include <deque>
#include <thread>
#include <mutex>

namespace meta
{

struct ThreadPool::Descriptor
{
    // The executor threads of the pool.
    std::vector<std::thread> threads;
    // The scheduled jobs.
    std::deque<BaseJobPtr> jobs;
    // The running jobs.
    std::deque<BaseJobPtr> scheduledJobs;
    // Locks the task queue.
    std::mutex queueLock;
    // Threads wait on new tasks.
    std::condition_variable lockCondition;
    // The amount of threads to create.
    const std::size_t threadCount = 0u;
    // The number of idling threads.
    std::atomic_size_t idleThreadCount = 0u;
    // Tells the thread pool to stop executing.
    std::atomic_bool stopSignalled = false;
    // Whether the pool is running.
    std::atomic_bool isRunning = false;

    explicit Descriptor(std::size_t threadCount) :
        threadCount(threadCount)
    {
    }

    // Pop the job from the queue and put it into the list of scheduled jobs.
    BaseJobPtr scheduleNextJob()
    {
        UniqueLock lock(queueLock);
        auto condition = [this]()
        {
            return stopSignalled || !jobs.empty();
        };
        lockCondition.wait(lock, condition);

        if (jobs.empty())
        {
            return {};
        }

        auto job = jobs.front();
        jobs.pop_front();
        scheduledJobs.push_back(job);
        return job;
    }

    static void threadMain(ThreadPool* self)
    {
        // Increase idle thread count before starting the thread loop.
        ++self->descriptor->idleThreadCount;

        while (!self->descriptor->stopSignalled)
        {
            self->runNextJob();
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

void ThreadPool::runNextJob()
{
    auto currentJob = descriptor->scheduleNextJob();
    if (!descriptor->stopSignalled)
    {
        --descriptor->idleThreadCount;
        currentJob->schedule();
        ++descriptor->idleThreadCount;
    }

    if (!currentJob)
    {
        return;
    }

    {
        GuardLock lock(descriptor->queueLock);
        std::erase(descriptor->scheduledJobs, currentJob);
    }
    currentJob->complete();
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
}

void ThreadPool::stop()
{
    abortIfFail(descriptor->isRunning);

    // Signal stop call.
    descriptor->stopSignalled = true;
    {
        GuardLock lock(descriptor->queueLock);

        // Stop the queued jobs first.
        for (auto& job : descriptor->jobs)
        {
            std::dynamic_pointer_cast<Job>(job)->stop();
        }
        descriptor->jobs.clear();

        // Then stop the scheduled jobs.
        for (auto& job : descriptor->scheduledJobs)
        {
            std::dynamic_pointer_cast<Job>(job)->stop();
        }
        descriptor->scheduledJobs.clear();
    }

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

bool ThreadPool::tryScheduleJob(JobPtr job)
{
    if (descriptor->stopSignalled)
    {
        return false;
    }

    abortIfFail(job);    

    {
        GuardLock lock(descriptor->queueLock);
        auto baseJob = static_cast<BaseJob*>(job.get());
        if (!baseJob->canQueue())
        {
            return false;
        }
        descriptor->jobs.push_back(job);
        baseJob->queue();
    }

    descriptor->lockCondition.notify_one();

    return true;
}

std::size_t ThreadPool::tryScheduleJobs(std::vector<JobPtr> jobs)
{
    if (descriptor->stopSignalled)
    {
        return 0u;
    }

    abortIfFail(!jobs.empty());
    std::size_t result = 0u;

    {
        GuardLock lock(descriptor->queueLock);

        for (auto& job : jobs)
        {
            auto baseJob = static_cast<BaseJob*>(job.get());
            if (!baseJob->canQueue())
            {
                continue;
            }
            descriptor->jobs.push_back(job);
            baseJob->queue();
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


bool async(JobPtr job)
{
    auto pool = Library::instance().threadPool();
    if (pool)
    {
        return pool->tryScheduleJob(job);
    }
    else
    {
        auto baseJob = static_cast<ThreadPool::BaseJob*>(job.get());

        baseJob->queue();
        baseJob->schedule();
        baseJob->complete();
        return true;
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

void yield(const std::chrono::nanoseconds& delay)
{
    auto pool = Library::instance().threadPool();
    if (pool)
    {
        pool->schedule(delay);
    }
}

} // namespace meta
