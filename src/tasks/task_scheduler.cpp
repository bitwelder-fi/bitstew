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

#include <meta/tasks/task_scheduler.hpp>
#include <assert.hpp>
#include <utils/scope_value.hpp>

#include <algorithm>

#include "task_scheduler_private.h"

namespace meta
{

class TaskScheduler::TaskSchedulerPrivate
{
public:
    static TaskPtr dequeueTask(TaskScheduler& self)
    {
        auto task = self.m_tasks.front();
        self.m_tasks.pop_front();
        detail::TaskPrivate::notifyTaskScheduled(*task);
        return task;
    }

    static TaskPtr popTask(TaskScheduler& self)
    {
        UniqueLock lock(self.m_queueLock);
        auto condition = [&self]()
        {
            // Bail out if there is a task to schedule, or stop thread is set.
            return self.m_stopThread || self.m_stopSignalled || !self.m_tasks.empty();
        };
        self.m_lockCondition.wait(lock, condition);

        if (self.m_tasks.empty())
        {
            return {};
        }

        auto task = dequeueTask(self);
        // Move the task to the running list.
        self.m_runningTasks.push_back(task);

        return task;
    }

    static void removeRunningTask(TaskScheduler& self, TaskPtr task)
    {
        UniqueLock lock(self.m_queueLock);
        auto it = std::find(self.m_runningTasks.begin(), self.m_runningTasks.end(), task);
        if (it != self.m_runningTasks.end())
        {
            self.m_runningTasks.erase(it);
        }
    }

    static void stopRunningTasks(TaskScheduler& self)
    {
        UniqueLock lock(self.m_queueLock);

        // Stop the running tasks.
        for (auto& task : self.m_runningTasks)
        {
            task->stop();
        }
    }

    static bool hasRunningTasks(TaskScheduler& self)
    {
        UniqueLock lock(self.m_queueLock);
        return !self.m_tasks.empty();
    }

    static void runSingleTask(TaskScheduler& self)
    {
        auto currentTask = popTask(self);
        if (!currentTask)
        {
            return;
        }
        if (self.m_stopThread)
        {
            return;
        }

        --self.m_idleThreadCount;
        detail::TaskPrivate::runTask(*currentTask);
        ++self.m_idleThreadCount;

        removeRunningTask(self, currentTask);
    }

    static void threadMain(TaskScheduler* self)
    {
        // Increase idle thread count before starting the thread loop.
        ++self->m_idleThreadCount;

        while (!self->m_stopThread)
        {
            runSingleTask(*self);
        }

        // Decrease idle thread count before exiting the thread loop.
        --self->m_idleThreadCount;
    }
};


TaskScheduler::TaskScheduler(size_t threadCount) :
#ifndef CONFIG_MULTI_THREADED
    m_threadCount(0u)
#else
    m_threadCount(threadCount)
#endif
{
    MAYBE_UNUSED(threadCount);
}

TaskScheduler::~TaskScheduler()
{
    abortIfFail(!m_isRunning);
}

void TaskScheduler::start()
{
    abortIfFail(!m_isRunning);
    m_stopSignalled = false;
    m_stopThread = false;
    m_idleThreadCount = 0u;

    m_threads.reserve(m_threadCount);
    for (size_t i = 0u; i < m_threadCount; ++i)
    {
        m_threads.push_back(Thread(&TaskSchedulerPrivate::threadMain, this));
        schedule();
    }
    m_isRunning = true;
    // Wait till the threads are all up and running.
    while (m_idleThreadCount < m_threadCount)
    {
        schedule(std::chrono::milliseconds(10));
    }
}

void TaskScheduler::stop()
{
    abortIfFail(m_isRunning);

    // Signal stop call.
    m_stopSignalled = true;

    // Wait till all the tasks queued get scheduled.
    while (TaskSchedulerPrivate::hasRunningTasks(*this))
    {
        schedule(std::chrono::milliseconds(10));
    }

    TaskSchedulerPrivate::stopRunningTasks(*this);
    // Notify all the threads to stop executing their tasks.
    m_lockCondition.notify_all();

    // Wait till threads complete the tasks.
    while (m_idleThreadCount < m_threadCount)
    {
        schedule(std::chrono::milliseconds(10));
    }

    // Stop the threads. Stops scheduling.
    m_stopThread = true;
    m_lockCondition.notify_all();

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

bool TaskScheduler::isBusy()
{
    bool busy = false;
    {
        UniqueLock lock(m_queueLock);
        busy = !m_tasks.empty() || !m_runningTasks.empty();
    }
    return busy;
}

TaskCompletionWatchObject TaskScheduler::tryQueueTask(TaskPtr task)
{
    abortIfFail(task);
    const auto taskStatus = task->getStatus();
    abortIfFail(taskStatus == Task::Status::Deferred || taskStatus == Task::Status::Stopped);

    if (m_stopSignalled)
    {
        return TaskCompletionWatchObject();
    }

    {
        GuardLock lock(m_queueLock);
        m_tasks.push_back(task);
        detail::TaskPrivate::notifyTaskQueued(*task, this);
    }

    m_lockCondition.notify_one();

    return task->getCompletionWatchObject();
}

std::vector<TaskCompletionWatchObject> TaskScheduler::tryQueueTasks(std::vector<TaskPtr> tasks)
{
    abortIfFail(!tasks.empty());
    if (m_stopSignalled)
    {
        return {};
    }

    std::vector<TaskCompletionWatchObject> result;

    {
        GuardLock lock(m_queueLock);

        m_tasks.insert(m_tasks.end(), tasks.begin(), tasks.end());
        for (auto& task : tasks)
        {
            detail::TaskPrivate::notifyTaskQueued(*task, this);
            result.push_back(task->getCompletionWatchObject());
        }
    }

    if (tasks.size() > 1)
    {
        m_lockCondition.notify_all();
    }
    else
    {
        m_lockCondition.notify_one();
    }

    return result;
}

size_t TaskScheduler::getTaskCount() const
{
    return m_tasks.size();
}

void TaskScheduler::schedule()
{
    schedule(std::chrono::nanoseconds(0));
}

void TaskScheduler::schedule(const std::chrono::nanoseconds& delay)
{
#if defined(CONFIG_MULTI_THREADED)
    std::this_thread::sleep_for(delay);
#else
    MAYBE_UNUSED(delay);
    while (!m_tasks.empty())
    {
        TaskSchedulerPrivate::runSingleTask(*this);
    }
#endif
}

} // namespace meta
