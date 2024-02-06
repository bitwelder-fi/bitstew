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

#ifndef META_JOB_HPP
#define META_JOB_HPP

#include <meta/meta_api.hpp>

#include <atomic>
#include <memory>

namespace meta
{

namespace detail
{
class JobPrivate;
}
class ThreadPool;

/// Represents a job executed by the thread pool. Derive from this to write jobs or queues.
///
/// To create a simple job:
/// \code
/// class SimpleJob : public Job
/// {
/// protected:
///     //...
///     void run() override
///     {
///         // put your code here.
///     }
/// };
/// \endcode
///
/// To create a job with a queue, you must put a loop to process the queue. Queues by nature require
/// a loop, which processes the queued data. This loop locks are long running tasks, which lock the
/// job to the thread which runs the job. Locked threads cannot be stopped by the thread pool, however
/// the thread pool signals the running jobs to stop. You should override the stopOverride() method,
/// and ensure the queue loop gets stopped, and the job can be closed.
/// \code
/// class SimpleJob : public Job
/// {
/// public:
///     void push(std::string text)
///     {
///         {
///             std::lock_guard<std::mutex> lock(guard);
///             queue.push(text);
///         }
///         signal.notify_one();
///     }
///
/// protected:
///     //...
///     void run() override
///     {
///         while (!isStopped())
///         {
///             std::unique_lock<std::mutex> lock(guard);
///             auto condition = [this]()
///             {
///                 return isStopped() || !this->m_queue.empty();
///             };
///             signal.wait(lock, condition);
///             if (queue.empty())
///             {
///                 continue;
///             }
///
///             while (!queue.empty())
///             {
///                 // Process the queued data.
///             }
///         }
///     }
///
///     void stopOverride() override
///     {
///         // Notifies the wait in the run() method to wake up and check the conditiom.
///         signal.notify_all();
///     }
///
///     std::mutex guard;
///     std::queue<std::string> queue;
///     std::condition_variable signal;
/// };
/// \endcode
///
/// you must make sure you can stop the job when the thread pool gets
/// signalled to stop the threads.
class META_API Job : public std::enable_shared_from_this<Job>
{
public:
    /// The status of the job.
    enum class Status
    {
        /// The job is deferred.
        Deferred,
        /// The job is queued for excecution.
        Queued,
        /// The job is scheduled, but not yet running.
        Scheduled,
        /// The job is running.
        Running,
        /// The job is stopped.
        Stopped
    };

    /// Destructor.
    virtual ~Job();

    /// Returns the status of the job.
    /// \return The status of the job.
    Status getStatus() const;

    /// Stops the job. Call this to schedule the downwinding of a job. To make this effective, you
    /// must check the stopped state of the job.
    void stop();

    /// Returns whether the stop got signalled on the job.
    bool isStopped() const;

    /// Waits for the job to complete.
    void wait();

protected:
    /// Constructor.
    explicit Job();

    /// Resets the job for reuse. Do not call this while the job is running.
    void reset();

    /// The main function of the job. Override this to implement job specific logic.
    virtual void run() = 0;

    /// Override this to implement job specific stop logic.
    virtual void stopOverride(){}

    /// Called by the thread pool when the job gets queued.
    virtual void onTaskQueued(){}
    /// Called by the thread pool when the job gets scheduled for execution.
    virtual void onJobScheduled(){}
    /// Called by the the thread pool when the job gets completed.
    virtual void onTaskCompleted(){}

    /// Sets the status of the job.
    /// \param status The status to set.
    void setStatus(Status status);

private:
    friend class detail::JobPrivate;
    std::unique_ptr<detail::JobPrivate> descriptor;
};

}

#endif // META_JOB_HPP

