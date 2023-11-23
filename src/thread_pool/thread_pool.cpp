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

#include <meta/thread_pool/thread_pool.hpp>
#include <assert.hpp>
#include <utils/scope_value.hpp>

#include <condition_variable>
#include <future>
#include <thread>
#include <queue>

namespace meta
{
namespace thread_pool
{

namespace
{

using UniqueLock = std::unique_lock<std::mutex>;
using GuardLock = std::lock_guard<std::mutex>;

}

class TaskPrivate
{
public:
    static void notifyTaskQueued(Task& self, ThreadPool* pool)
    {
        self.m_threadPool = pool;
        self.setStatus(Task::Status::Queued);
        self.notifyTaskQueued();
    }

    static void notifyTaskDequeued(Task& self)
    {
        self.notifyTaskDequeued();
        self.m_threadPool = nullptr;
    }
};

Task::~Task()
{
    abortIfFail(m_status == Status::Stopped);
}

void Task::stop()
{
    m_stopSignalled = true;
    stopOverride();
}

TaskFuture Task::getFuture()
{
    return m_completed.get_future();
}

void Task::run()
{
    abortIfFail(m_status == Status::Queued);

    m_owningThread = std::this_thread::get_id();
    setStatus(Status::Running);

    if (!m_stopSignalled)
    {
        runOverride();
    }

    setStatus(Status::Stopped);
    m_completed.set_value();
}


class ThreadPool::ThreadPoolPrivate
{
public:
    static TaskFuture pushTask(ThreadPool& self, TaskPtr task)
    {
        self.m_tasks.push(task);
        TaskPrivate::notifyTaskQueued(*task, &self);
        return task->getFuture();
    }

    static TaskPtr popTask(ThreadPool& self)
    {
        UniqueLock lock(self.m_queueLock);
        auto condition = [&self]()
        {
            return !self.m_tasks.empty() || self.m_stop;
        };
        self.m_lockCondition.wait(lock, condition);

        if (self.m_tasks.empty())
        {
            return {};
        }

        auto task = self.m_tasks.front();
        self.m_tasks.pop();

        // Add the task to the running list.
        self.m_runningTasks.push_back(task);

        return task;
    }

    static void removeRunningTask(ThreadPool& self, TaskPtr task)
    {
        UniqueLock lock(self.m_queueLock);
        auto predicate = [task](auto& it)
        {
            auto lock = it.lock();
            return lock && lock == task;
        };
        auto it = std::find_if(self.m_runningTasks.begin(), self.m_runningTasks.end(), predicate);
        if (it != self.m_runningTasks.end())
        {
            self.m_runningTasks.erase(it);
            TaskPrivate::notifyTaskDequeued(*task);
        }
    }

    static void stopAndSignalTasks(ThreadPool& self)
    {
        UniqueLock lock(self.m_queueLock);
        self.m_stop = true;
        // Pop queued tasks and stop them. Those should not be executed anymore.
        while (!self.m_tasks.empty())
        {
            auto task = self.m_tasks.front();
            self.m_tasks.pop();
            task->stop();
        }
        // Stop the running tasks too.
        for (auto& task : self.m_runningTasks)
        {
            auto locked = task.lock();
            if (!locked)
            {
                continue;
            }
            locked->stop();
        }
    }

    static void threadMain(ThreadPool* self)
    {
        // Increase idle thread count before starting the thread loop.
        ++self->m_idleThreadCount;

        while (!self->m_stop)
        {
            auto currentTask = ThreadPoolPrivate::popTask(*self);
            if (!currentTask)
            {
                continue;
            }
            if (self->m_stop)
            {
                break;
            }

            --self->m_idleThreadCount;
            currentTask->run();
            ++self->m_idleThreadCount;

            ThreadPoolPrivate::removeRunningTask(*self, currentTask);
        }

        // Decrease idle thread count before exiting the thread loop.
        --self->m_idleThreadCount;
    }

};


ThreadPool::~ThreadPool()
{
    abortIfFail(!m_isRunning);
}

void ThreadPool::start()
{
    abortIfFail(!m_isRunning);
    m_stop = false;
    m_idleThreadCount = 0u;

    m_threads.reserve(m_threadCount);
    for (size_t i = 0u; i < m_threadCount; ++i)
    {
        m_threads.push_back(std::thread(&ThreadPoolPrivate::threadMain, this));
        std::this_thread::yield();
    }
    m_isRunning = true;
    // Wait till the threads are all up and running.
    while (m_idleThreadCount < m_threadCount)
    {
        std::this_thread::yield();
    }
}

void ThreadPool::stop()
{
    abortIfFail(m_isRunning);

    ThreadPoolPrivate::stopAndSignalTasks(*this);
    // Notify all the threads to stop executing their tasks.
    m_lockCondition.notify_all();

    // Yield till thread idle count gets back to 0. This ensures the threads are out of their main.
    while (m_idleThreadCount > 0u)
    {
        std::this_thread::yield();
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

TaskFuture ThreadPool::addTask(TaskPtr task)
{
    abortIfFail(task);

    TaskFuture result;
    {
        GuardLock lock(m_queueLock);
        result = ThreadPoolPrivate::pushTask(*this, task);
    }

    m_lockCondition.notify_one();
    std::this_thread::yield();

    return result;
}

std::vector<TaskFuture> ThreadPool::addTasks(std::vector<TaskPtr> tasks)
{
    abortIfFail(!tasks.empty());
    std::vector<TaskFuture> result;

    for (auto& task : tasks)
    {
        auto future = addTask(task);
        result.push_back(std::move(future));
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

bool ThreadPool::isBusy()
{
    bool busy = false;
    {
        UniqueLock lock(m_queueLock);
        busy = !m_tasks.empty() || !m_runningTasks.empty();
    }
    return busy;
}

}} // namespace meta::thread_pool
