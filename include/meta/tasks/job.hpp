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
#include <meta/threading.hpp>

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
    inline Status getStatus() const
    {
        return m_status.load();
    }

    /// Returns the thread which runs the job.
    /// \return The identifier of the thread which runs the job.
    inline ThreadId getOwningThread() const
    {
        return m_owningThread;
    }

    /// Stops the job. Call this to schedule the downinding of the job.
    void stop();

    /// Returns whether the stop got signalled on the job.
    inline bool isStopped() const
    {
        return m_stopSignalled;
    }

    /// Returns the future object which notifies the completion of the job.
    /// \return The future object of the job. Use this to wait for the job completion.
    JobFuture getFuture();

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
    inline void setStatus(Status status)
    {
        m_status = status;
    }

private:
    friend class detail::JobPrivate;

    // The worker task.
    PackagedTask<void(Job&)> m_worker;
    // The thread which owns the job.
    ThreadId m_owningThread;
    // The job status.
    Atomic<Status> m_status = Status::Deferred;
    // Holds the signalled stop.
    AtomicBool m_stopSignalled = false;
};

}

#endif // META_JOB_HPP

