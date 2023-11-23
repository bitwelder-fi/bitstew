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

#include <condition_variable>
#include <future>
#include <mutex>
#include <thread>
#include <queue>

namespace meta{ namespace thread_pool
{

class Task;
class ThreadPool;

using TaskPtr = std::shared_ptr<Task>;
using TaskWeakPtr = std::weak_ptr<Task>;
using TaskFuture = std::future<void>;

class TaskPrivate;
/// Represents a job executed by the thread pool.
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
    std::thread::id getOwningThread() const
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
    virtual void runOverride()
    {
    }

    /// Override this to implement task specific stop logic.
    virtual void stopOverride()
    {
    }

    /// Called by thread pool when the task is queued. Override this to implement task specific logic.
    virtual void notifyTaskQueued()
    {
    }

    /// Called by thread pool when the task is dequeued. Override this to implement task specific logic.
    virtual void notifyTaskDequeued()
    {
    }

    /// Sets the status of the task.
    /// \param status The status to set.
    void setStatus(Status status)
    {
        m_status = status;
    }

    /// The thread pool to which the task is queued.
    ThreadPool* m_threadPool = nullptr;

private:
    friend class TaskPrivate;
    // The thread which owns the task.
    std::thread::id m_owningThread;
    // Promise for completion.
    std::promise<void> m_completed;
    // The task status.
    std::atomic<Status> m_status = Status::Deferred;
    // Holds the signalled stop.
    std::atomic_bool m_stopSignalled = false;
};

/// The thread pool. Queues and distributes tasks to available threads.
class META_API ThreadPool
{
public:
    /// Constructor. Creates the thread pool.
    explicit ThreadPool(size_t threadCount) :
        m_threadCount(threadCount)
    {
    }
    /// Destructor. Stops and joins the threads.
    ~ThreadPool();

    /// Starts the thread pool.
    void start();

    /// Stops the thread pool.
    void stop();

    /// Returns whether the thread pool is running.
    /// \return If the thread pool is running, returns \e true, otherwise \e false.
    bool isRunning() const
    {
        return m_isRunning;
    }
    /// Returns whether the thread pool got signalled to stop.
    /// \return If the thread pool was signalled to stop, returns \e true, otherwise \e false.
    bool isStopped() const
    {
        return m_stop;
    }

    /// Returns the number of running threads of a thread pool.
    /// \return The number of threads running. If the thread pool is stopped, returns 0u.
    size_t getThreadCount() const
    {
        return m_isRunning ? m_threadCount : 0u;
    }

    /// Returns the number of idle threads of the thread pool.
    /// \return The number of idle threads.
    size_t getIdleCount() const
    {
        return m_idleThreadCount;
    }

    /// Queues a task to for execution.
    /// \param task The task queue for execution.
    /// \returns The future object of the task.
    TaskFuture addTask(TaskPtr task);

    /// Queue multiple tasks for execution.
    /// \param tasks The tasks to queue for execution.
    /// \returns The futures objects of the queued tasks.
    std::vector<TaskFuture> addTasks(std::vector<TaskPtr> tasks);

    /// Returns whether the thread pool is busy executing tasks.
    /// \retirn If the thread pool is executing tasks, returns \e true, otherwise \e false.
    bool isBusy();

private:
    class ThreadPoolPrivate;

    // The amount of threads to create.
    const size_t m_threadCount = 0u;
    // The number of idling threads.
    std::atomic<size_t> m_idleThreadCount = 0u;
    // Locks the task queue.
    std::mutex m_queueLock;
    // Threads wait on new tasks.
    std::condition_variable m_lockCondition;
    // The scheduled tasks.
    std::queue<TaskPtr> m_tasks;
    // The running tasks.
    std::vector<TaskWeakPtr> m_runningTasks;
    // The executor threads of the pool.
    std::vector<std::thread> m_threads;
    // Tells the thread pool to stop executing.
    std::atomic_bool m_stop = false;
    // Whether the pool is running.
    bool m_isRunning = false;
};

}} // namespace meta::thread_pool

#endif // META_THREAD_POOL_HPP
