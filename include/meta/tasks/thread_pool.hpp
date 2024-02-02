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

#ifndef META_THREAD_POOL_HPP
#define META_THREAD_POOL_HPP

#include <meta/meta.hpp>
#include <meta/threading.hpp>

#include <chrono>
#include <deque>
#include <memory>
#include <vector>

namespace meta
{

class Job;
class ThreadPool;

using TaskPtr = std::shared_ptr<Job>;

/// The thread pool is responsible to dispatch worker jobs to threads in an efficien way. Though the
/// system allows you to have several thread pools per application, for efficiency, it is recommended
/// to have only one thread pool per application.
///
/// You must start the thread pool to run jobs. To do that, call the start() method.
///
/// To queue a worker job for execution, call pushJob() method. To queue a set of worker jobs for
/// execution, call pushMultipleJobs() method. Both methods return the futures of the queued workers.
/// You can use these future objects to wait for the job completion.
///
/// In multi-threaded environments, the thread pool schedules jobs automatically. In single-
/// threaded environment however, you must call schedule() method to run the jobs. When you call
/// schedule() in multi-threaded environment, that will result in yielding.
///
/// To stop the thread pool, call the stop() method. This makes the thread pool to stop accepting new
/// jobs, waits till all the queued jobs get scheduled, and stops the threads.
///
/// You should not push jobs which would hold a thread for the entire lifetime of the application. If
/// you need such scenarios, it is better to create dedicated threads for those.
///
/// You should recycle the jobs after their completion. A job should never be used twice.
class META_API ThreadPool
{
public:
    /// Constructor. Creates a thread pool with a number of threads. The argument is ignored in
    /// single-threaded environment.
    explicit ThreadPool(std::size_t threadCount);
    /// Destructor. Aborts the application if the thread pool is still running.
    ~ThreadPool();

    /// Starts the thread pool.
    void start();

    /// Stops the thread pool.
    void stop();

    /// Returns whether the thread pool is busy executing jobs.
    /// \return If the thread pool is executing jobs, returns \e true, otherwise \e false.
    bool isBusy();

    /// Returns whether the thread pool is running.
    /// \return If the thread pool is running, returns \e true, otherwise \e false.
    bool isRunning() const
    {
        return m_isRunning;
    }

    /// Returns whether the thread pool got signalled to stop.
    /// \return If the thread pool got signalled to stop, returns \e true, otherwise \e false.
    bool isStopSignalled() const
    {
        return m_stopSignalled;
    }

    /// Returns whether the thread pool stopped its threads.
    /// \return If the thread pool stopped its threads, returns \e true, otherwise \e false.
    bool isScheduleStopped() const
    {
        return m_stopThread;
    }

    /// Returns the number of running threads of a thread pool.
    /// \return The number of threads running. If the thread pool is stopped, returns 0u.
    std::size_t getThreadCount() const
    {
        return m_isRunning ? m_threadCount : 0u;
    }

    /// Returns the number of idle threads of the thread pool.
    /// \return The number of idle threads.
    std::size_t getIdleCount() const
    {
        return m_idleThreadCount;
    }

    /// Queues a job to for execution.
    /// \param job The job to queue for execution.
    /// \returns The future object of the task. If the thread pool is stopped, returns an invalid
    ///          task future.
    TaskCompletionWatchObject pushJob(TaskPtr job);

    /// Queue multiple jobs for execution.
    /// \param jobs The jobs to queue for execution.
    /// \returns The futures objects of the queued tasks. If the thread pool is stopped, returns
    ///          invalid task futures.
    std::vector<TaskCompletionWatchObject> pushMultipleJobs(std::vector<TaskPtr> jobs);

    /// Returns the queued job count.
    /// \return The queued job count.
    std::size_t getQueuedJobs() const;

    /// Schedules the jobs queued. On single multi-threaded environment, the function yields the
    /// current thread. On single-threaded environment, executes the queued jobs.
    void schedule();

    /// Schedules the jobs queued after a delay. On single multi-threaded environment, the function
    /// yields the current thread after a given delay. On single-threaded environment, executes the
    /// queued tasks.
    /// \param delay The delay after which to schedule the jobs.
    void schedule(const std::chrono::nanoseconds& delay);

private:
    class ThreadPoolPrivate;

    // The amount of threads to create.
    const std::size_t m_threadCount = 0u;
    // The number of idling threads.
    Atomic<std::size_t> m_idleThreadCount = 0u;
    // Locks the task queue.
    Mutex m_queueLock;
    // Threads wait on new tasks.
    ConditionVariable m_lockCondition;
    // The scheduled tasks.
    std::deque<TaskPtr> m_tasks;
    // The running tasks.
    std::vector<TaskPtr> m_runningTasks;
    // The executor threads of the pool.
    std::vector<Thread> m_threads;
    // Tells the thread pool to stop executing.
    AtomicBool m_stopSignalled = false;
    // Tells the thread pool to stop executing.
    AtomicBool m_stopThread = false;
    // Whether the pool is running.
    bool m_isRunning = false;
};

} // namespace meta

#endif // META_THREAD_POOL_HPP
