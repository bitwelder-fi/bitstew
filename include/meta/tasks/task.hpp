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

#ifndef META_TASK_HPP
#define META_TASK_HPP

#include <meta/meta_api.hpp>
#include <meta/threading.hpp>

namespace meta
{

namespace detail
{
class TaskPrivate;
}
class TaskScheduler;

/// Represents a job executed by the task scheduler.
class META_API Task : public std::enable_shared_from_this<Task>
{
public:
    /// The status of the task.
    enum class Status
    {
        /// The task is deferred.
        Deferred,
        /// The task is queued for excecution.
        Queued,
        /// The task is scheduled, but not yet running.
        Scheduled,
        /// The task is running.
        Running,
        /// The task is idling.
        Idle,
        /// The task is stopped.
        Stopped
    };

    /// Destructor.
    virtual ~Task();

    /// Returns the status of the task.
    /// \return The status of the task.
    Status getStatus() const
    {
        return m_status.load();
    }

    /// Returns the thread which runs the task.
    /// \return The identifier of the thread which runs the task.
    ThreadId getOwningThread() const
    {
        return m_owningThread;
    }

    /// Executes the task.
    void run();

    /// Stops the task. Call this to schedule the downinding of the task.
    void stop();

    /// Returns whether the stop got signalled on the task.
    bool isStopped() const
    {
        return m_stopSignalled;
    }

    /// Returns the future object which notifies the completion of the task.
    /// \return The future object of the task. Use this to wait for the task completion.
    TaskFuture getFuture();

protected:
    /// Constructor.
    explicit Task() = default;

    /// The main function of the task. Override this to implement task specific logic.
    virtual void runOverride(){}

    /// Override this to implement task specific stop logic.
    virtual void stopOverride(){}

    /// Called by task scheduler when the task is queued.
    virtual void onTaskQueued(){}
    /// Called by task scheduler when the task is scheduled and removed from the task scheduler queue.
    virtual void onTaskScheduled(){}
    /// Called by the task when the task is completed. Override this to implement task specific logic.
    virtual void onTaskCompleted(){}

    /// Sets the status of the task.
    /// \param status The status to set.
    void setStatus(Status status)
    {
        m_status = status;
    }

    /// The task scheduler to which the task is queued.
    TaskScheduler* m_taskScheduler = nullptr;
    // Holds the signalled stop.
    AtomicBool m_stopSignalled = false;

private:
    friend class detail::TaskPrivate;
    // The thread which owns the task.
    ThreadId m_owningThread;
    // TaskPromise for completion.
    TaskPromise m_completed;
    // The task status.
    Atomic<Status> m_status = Status::Deferred;
};

}

#endif // META_TASK_HPP

