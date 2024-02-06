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

#include <chrono>
#include <memory>
#include <vector>

namespace meta
{

class Job;
class ThreadPool;

using JobPtr = std::shared_ptr<Job>;


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
    bool isRunning() const;

    /// Returns whether the thread pool got signalled to stop.
    /// \return If the thread pool got signalled to stop, returns \e true, otherwise \e false.
    bool isStopSignalled() const;

    /// Returns the number of running threads of a thread pool.
    /// \return The number of threads running. If the thread pool is stopped, returns 0u.
    std::size_t getThreadCount() const;

    /// Returns the number of idle threads of the thread pool.
    /// \return The number of idle threads.
    std::size_t getIdleCount() const;

    /// Queues a job to for execution.
    /// \param job The job to queue for execution.
    /// \returns If the job was queued with success. returns \e true, otherwise \e false.
    bool pushJob(JobPtr job);

    /// Queue multiple jobs for execution.
    /// \param jobs The jobs to queue for execution.
    /// \returns The number of jobs pushed with success.
    std::size_t pushMultipleJobs(std::vector<JobPtr> jobs);

    /// Returns the queued job count.
    /// \return The queued job count.
    std::size_t getQueuedJobs() const;

    /// Schedules the jobs queued. On single multi-threaded environment, the function yields the
    /// current thread. On single-threaded environment, executes the queued jobs.
    void schedule();

    /// Schedules the jobs queued with a delay. On single multi-threaded environment, the function
    /// yields the current thread with a given delay. On single-threaded environment, executes the
    /// queued tasks.
    /// \param delay The delay after which to schedule the jobs.
    void schedule(const std::chrono::nanoseconds& delay);

private:
    class Descriptor;
    std::unique_ptr<Descriptor> descriptor;
};


/// Executes the job asynchronously. To wait for the job completion, call Job::wait() methods.
/// \param job The job to execute.
void META_API async(JobPtr job);

/// Yields the meta thread pool of the library.
void META_API yield();

} // namespace meta

#endif // META_THREAD_POOL_HPP
