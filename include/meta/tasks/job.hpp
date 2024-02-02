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
class WorkerPrivate;
}
class ThreadPool;

/// Represents a job executed by the thread pool.
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
        /// The job is idling.
        Idle,
        /// The job is stopped.
        Stopped
    };

    /// Destructor.
    virtual ~Job();

    /// Returns the status of the job.
    /// \return The status of the job.
    Status getStatus() const
    {
        return m_status.load();
    }

    /// Returns the thread which runs the job.
    /// \return The identifier of the thread which runs the job.
    ThreadId getOwningThread() const
    {
        return m_owningThread;
    }

    /// Executes the job.
    void run();

    /// Stops the job. Call this to schedule the downinding of the job.
    void stop();

    /// Returns whether the stop got signalled on the job.
    bool isStopped() const
    {
        return m_stopSignalled;
    }

    /// Returns the future object which notifies the completion of the job.
    /// \return The future object of the job. Use this to wait for the job completion.
    TaskCompletionWatchObject getCompletionWatchObject();

protected:
    /// Constructor.
    explicit Job();

    /// Resets the job for reuse.
    void reset();

    /// The main function of the job. Override this to implement job specific logic.
    virtual void runOverride(){}

    /// Override this to implement job specific stop logic.
    virtual void stopOverride(){}

    /// Called by job scheduler when the job is queued.
    virtual void onTaskQueued(){}
    /// Called by job scheduler when the job is scheduled and removed from the job scheduler queue.
    virtual void onTaskScheduled(){}
    /// Called by the job when the job is completed. Override this to implement job specific logic.
    virtual void onTaskCompleted(){}

    /// Sets the status of the job.
    /// \param status The status to set.
    void setStatus(Status status)
    {
        m_status = status;
    }

    /// The job scheduler to which the job is queued.
    ThreadPool* m_threadPool = nullptr;
    // Holds the signalled stop.
    AtomicBool m_stopSignalled = false;

private:
    friend class detail::WorkerPrivate;
    // The thread which owns the job.
    ThreadId m_owningThread;
    // TaskCompletionSignal for completion.
    TaskCompletionSignal m_completed;
    // The job status.
    Atomic<Status> m_status = Status::Deferred;
};

}

#endif // META_JOB_HPP

