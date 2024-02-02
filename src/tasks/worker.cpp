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

#include <assert.hpp>
#include <meta/tasks/worker.hpp>

#include "../private/thread_pool.hpp"

namespace meta
{

namespace detail
{

void TaskPrivate::notifyTaskQueued(Task& self, TaskScheduler* pool)
{
    self.m_taskScheduler = pool;
    self.setStatus(Task::Status::Queued);
    self.onTaskQueued();
}

void TaskPrivate::notifyTaskScheduled(Task& self)
{
    self.setStatus(Task::Status::Scheduled);
    self.onTaskScheduled();
}

bool TaskPrivate::isTaskQueued(Task& self)
{
    return self.m_taskScheduler != nullptr;
}

void TaskPrivate::runTask(Task& self)
{
    abortIfFail(self.m_status == Task::Status::Scheduled);

    self.setStatus(Task::Status::Running);
    self.m_owningThread = ThisThread::get_id();

    if ((self.m_taskScheduler && !self.m_taskScheduler->isStopSignalled()) || !self.m_stopSignalled)
    {
        self.runOverride();
    }

    self.setStatus(Task::Status::Stopped);
    self.m_completed.set_value();

    // Create a new promise as the current one is already consumed. Refetching the future of a promise
    // throws exception.
    self.reset();
    self.m_taskScheduler = nullptr;
    self.onTaskCompleted();
}

} // namespace detail


Task::Task()
{
    reset();
}

Task::~Task()
{
    abortIfFail(m_status == Status::Deferred || m_status == Status::Stopped);
}

void Task::reset()
{
    TaskCompletionSignal signal;
    m_completed.swap(signal);
}

void Task::run()
{
    detail::TaskPrivate::runTask(*this);
}

void Task::stop()
{
    m_stopSignalled = true;
    stopOverride();
}

TaskCompletionWatchObject Task::getCompletionWatchObject()
{
    return m_completed.get_future();
}

} // namespace meta
