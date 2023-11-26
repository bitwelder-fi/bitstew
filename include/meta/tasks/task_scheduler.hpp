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

#ifndef META_TASK_SCHEDULER_HPP
#define META_TASK_SCHEDULER_HPP

#include <meta/meta.hpp>
#include <meta/threading.hpp>

#include <deque>

namespace meta
{

class Task;
class TaskScheduler;

using TaskPtr = std::shared_ptr<Task>;
using TaskWeakPtr = std::weak_ptr<Task>;

/// The task scheduler queues and distributes tasks to available threads. Though the system allows you
/// to have several task schedulers per application, it is recommended to use only one instance of
/// task scheduler in an application.
///
/// You must start the task scheduler to run tasks. To start the task scheduler, call start() method.
///
/// To queue a task for execution, call tryQueueTask() method. To queue a set of tasks for execution,
/// call tryQueueTasks() method. Both methods return the futures of the queued task. You can use these
/// future objects to wait for the task completion.
///
/// In multi-threaded environments, the task scheduler schedules tasks automatically. In single-
/// threaded environment however. you must call schedule() method to run the tasks. When you call
/// schedule() in multi-threaded environment, that will result in yielding.
///
/// To stop the task scheduler, call stop() method. This signals the scheduler to stop accepting new
/// tasks, waits till all the queued tasks get scheduled, and stops the threads.
///
/// If your task holds a thread for its entire lifetime, you should ensure that you release the thread
/// when the task scheduler signals the thread to stop.
///
/// Try to avoid task rescheduling. If you do, make sure that the task is in Task::State::Stopped state.
class META_API TaskScheduler
{
public:
    /// Constructor. Creates a task scheduler with a number of threads. The argument is ignored in
    /// single-threaded environment.
    explicit TaskScheduler(size_t threadCount);
    /// Destructor. Aborts the application if the task scheduler is still running.
    ~TaskScheduler();

    /// Starts the task scheduler threads.
    void start();

    /// Stops the task scheduler.
    void stop();

    /// Returns whether the task scheduler is busy executing tasks.
    /// \retirn If the task scheduler is executing tasks, returns \e true, otherwise \e false.
    bool isBusy();

    /// Returns whether the task scheduler is running.
    /// \return If the task scheduler is running, returns \e true, otherwise \e false.
    bool isRunning() const
    {
        return m_isRunning;
    }
    /// Returns whether the task scheduler got signalled to stop.
    /// \return If the task scheduler got signalled to stop, returns \e true, otherwise \e false.
    bool isStopSignalled() const
    {
        return m_stopSignalled;
    }

    /// Returns whether the task scheduler stopped its threads.
    /// \return If the task scheduler stopped its threads, returns \e true, otherwise \e false.
    bool isScheduleStopped() const
    {
        return m_stopThread;
    }

    /// Returns the number of running threads of a task scheduler.
    /// \return The number of threads running. If the task scheduler is stopped, returns 0u.
    size_t getThreadCount() const
    {
        return m_isRunning ? m_threadCount : 0u;
    }

    /// Returns the number of idle threads of the task scheduler.
    /// \return The number of idle threads.
    size_t getIdleCount() const
    {
        return m_idleThreadCount;
    }

    /// Queues a task to for execution.
    /// \param task The task queue for execution.
    /// \returns The future object of the task. If the task scheduler is stopped, returns an invalid
    ///          task future.
    TaskFuture tryQueueTask(TaskPtr task);

    /// Queue multiple tasks for execution.
    /// \param tasks The tasks to queue for execution.
    /// \returns The futures objects of the queued tasks. If the task scheduler is stopped, returns
    ///          invalid task futures.
    std::vector<TaskFuture> tryQueueTasks(std::vector<TaskPtr> tasks);

    /// Returns the queued task count.
    /// \return The queued task count.
    size_t getTaskCount() const;

    /// Schedules the tasks queued. On single multi-threaded environment, the function yields the
    /// current thread. On single-threaded environment, executes the queued tasks.
    void schedule();

    /// Schedules the tasks queued after a delay. On single multi-threaded environment, the function
    /// yields the current thread after a given delay. On single-threaded environment, executes the
    /// queued tasks.
    /// \param delay The delay after which to schedule the tasks.
    void schedule(const std::chrono::nanoseconds& delay);

private:
    class TaskSchedulerPrivate;

    // The amount of threads to create.
    const size_t m_threadCount = 0u;
    // The number of idling threads.
    Atomic<size_t> m_idleThreadCount = 0u;
    // Locks the task queue.
    Mutex m_queueLock;
    // Threads wait on new tasks.
    ConditionVariable m_lockCondition;
    // The scheduled tasks.
    std::deque<TaskPtr> m_tasks;
    // The running tasks.
    std::vector<TaskWeakPtr> m_runningTasks;
    // The executor threads of the pool.
    std::vector<Thread> m_threads;
    // Tells the task scheduler to stop executing.
    AtomicBool m_stopSignalled = false;
    // Tells the task scheduler to stop executing.
    AtomicBool m_stopThread = false;
    // Whether the pool is running.
    bool m_isRunning = false;
};

} // namespace meta

#endif // META_TASK_SCHEDULER_HPP
