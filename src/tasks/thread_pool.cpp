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

#include <algorithm>

#include "../private/thread_pool.hpp"

namespace meta
{

class ThreadPool::ThreadPoolPrivate
{
public:
    static JobPtr dequeueJob(ThreadPool& self)
    {
        auto job = self.m_jobs.front();
        self.m_jobs.pop_front();
        detail::JobPrivate::notifyJobScheduled(*job, ThisThread::get_id());
        return job;
    }

    static JobPtr popJob(ThreadPool& self)
    {
        UniqueLock lock(self.m_queueLock);
        auto condition = [&self]()
        {
            // Bail out if there is a task to schedule, or stop thread is set.
            return self.m_stopSignalled || !self.m_jobs.empty();
        };
        self.m_lockCondition.wait(lock, condition);

        if (self.m_jobs.empty())
        {
            return {};
        }

        return dequeueJob(self);
    }

    static void stopJobs(ThreadPool& self)
    {
        GuardLock lock(self.m_queueLock);

        // Stop the queued jobs first.
        for (auto& job : self.m_jobs)
        {
            job->stop();
        }
        self.m_jobs.clear();

        // Then stop the scheduled jobs.
        for (auto& job :self.m_scheduledJobs)
        {
            job->stop();
        }
    }

    static bool hasRunningJobs(ThreadPool& self)
    {
        UniqueLock lock(self.m_queueLock);
        return !self.m_jobs.empty() || !self.m_scheduledJobs.empty() || self.m_idleThreadCount < self.m_threadCount;
    }

    static void runSingleJob(ThreadPool& self)
    {
        auto currentJob = popJob(self);
        if (!currentJob || self.m_stopSignalled || currentJob->isStopped())
        {
            return;
        }

        --self.m_idleThreadCount;
        {
            GuardLock lock(self.m_queueLock);
            self.m_scheduledJobs.push_back(currentJob);
        }
        detail::JobPrivate::runJob(*currentJob);

        {
            GuardLock lock(self.m_queueLock);
            std::erase(self.m_scheduledJobs, currentJob);
        }
        ++self.m_idleThreadCount;
    }

    static void threadMain(ThreadPool* self)
    {
        // Increase idle thread count before starting the thread loop.
        ++self->m_idleThreadCount;

        while (!self->m_stopSignalled)
        {
            runSingleJob(*self);
        }

        // Decrease idle thread count before exiting the thread loop.
        --self->m_idleThreadCount;
    }
};


ThreadPool::ThreadPool(std::size_t threadCount) :
#ifndef CONFIG_MULTI_THREADED
    m_threadCount(0u)
#else
    m_threadCount(threadCount)
#endif
{
    MAYBE_UNUSED(threadCount);
}

ThreadPool::~ThreadPool()
{
    abortIfFail(!m_isRunning);
}

void ThreadPool::start()
{
    abortIfFail(!m_isRunning);
    m_stopSignalled = false;
    m_idleThreadCount = 0u;

    m_threads.reserve(m_threadCount);
    for (std::size_t i = 0u; i < m_threadCount; ++i)
    {
        m_threads.push_back(Thread(&ThreadPoolPrivate::threadMain, this));
    }
    m_isRunning = true;
    // Wait till the threads are all up and running.
    while (m_idleThreadCount < m_threadCount)
    {
        schedule(std::chrono::milliseconds(10));
    }
}

void ThreadPool::stop()
{
    abortIfFail(m_isRunning);

    // Signal stop call.
    m_stopSignalled = true;
    ThreadPoolPrivate::stopJobs(*this);

    // Notify all the threads to stop executing their jobs.
    m_lockCondition.notify_all();
    schedule();

    // Wait till threads complete the tasks.
    // bool oneMoreTime = m_idleThreadCount > 0u;
    while (m_idleThreadCount > 0u)
    {
        schedule(std::chrono::milliseconds(10));
    }

    // if (oneMoreTime)
    {
        // There were few threads waiting.
        m_lockCondition.notify_all();
    }

    // Join the threads.
    for (auto& thread : m_threads)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }

    m_threads.clear();
    m_isRunning = false;
}

bool ThreadPool::isBusy()
{
    return ThreadPoolPrivate::hasRunningJobs(*this);
}

bool ThreadPool::pushJob(JobPtr job)
{
    if (m_stopSignalled)
    {
        return false;
    }

    abortIfFail(job);

    const auto jobStatus = job->getStatus();
    abortIfFail(jobStatus == Job::Status::Deferred || jobStatus == Job::Status::Stopped);

    {
        GuardLock lock(m_queueLock);
        m_jobs.push_back(job);
        detail::JobPrivate::notifyJobQueued(*job, this);
    }

    m_lockCondition.notify_one();

    return true;
}

std::size_t ThreadPool::pushMultipleJobs(std::vector<JobPtr> jobs)
{
    if (m_stopSignalled)
    {
        return 0u;
    }

    abortIfFail(!jobs.empty());
    std::size_t result = 0u;

    {
        GuardLock lock(m_queueLock);

        m_jobs.insert(m_jobs.end(), jobs.begin(), jobs.end());
        for (auto& job : jobs)
        {
            detail::JobPrivate::notifyJobQueued(*job, this);
            ++result;
        }
    }

    if (jobs.size() > 1)
    {
        m_lockCondition.notify_all();
    }
    else
    {
        m_lockCondition.notify_one();
    }

    return result;
}

std::size_t ThreadPool::getQueuedJobs() const
{
    return m_jobs.size();
}

void ThreadPool::schedule()
{
    schedule(std::chrono::nanoseconds(1));
}

void ThreadPool::schedule(const std::chrono::nanoseconds& delay)
{
#if defined(CONFIG_MULTI_THREADED)
    std::this_thread::sleep_for(delay);
#else
    MAYBE_UNUSED(delay);
    while (!m_jobs.empty())
    {
        ThreadPoolPrivate::runSingleJob(*this);
    }
#endif
}

} // namespace meta
