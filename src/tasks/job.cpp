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
#include <meta/tasks/job.hpp>

#include "../private/thread_pool.hpp"

namespace meta
{

namespace detail
{

void WorkerPrivate::notifyTaskQueued(Job& self, ThreadPool* pool)
{
    self.m_threadPool = pool;
    self.setStatus(Job::Status::Queued);
    self.onTaskQueued();
}

void WorkerPrivate::notifyTaskScheduled(Job& self)
{
    self.setStatus(Job::Status::Scheduled);
    self.onTaskScheduled();
}

bool WorkerPrivate::isTaskQueued(Job& self)
{
    return self.m_threadPool != nullptr;
}

void WorkerPrivate::runTask(Job& self)
{
    abortIfFail(self.m_status == Job::Status::Scheduled);

    self.setStatus(Job::Status::Running);
    self.m_owningThread = ThisThread::get_id();

    if ((self.m_threadPool && !self.m_threadPool->isStopSignalled()) || !self.m_stopSignalled)
    {
        self.runOverride();
    }

    self.setStatus(Job::Status::Stopped);
    self.m_completed.set_value();

    // Create a new promise as the current one is already consumed. Refetching the future of a promise
    // throws exception.
    self.reset();
    self.m_threadPool = nullptr;
    self.onTaskCompleted();
}

} // namespace detail


Job::Job()
{
    reset();
}

Job::~Job()
{
    abortIfFail(m_status == Status::Deferred || m_status == Status::Stopped);
}

void Job::reset()
{
    TaskCompletionSignal signal;
    m_completed.swap(signal);
}

void Job::run()
{
    detail::WorkerPrivate::runTask(*this);
}

void Job::stop()
{
    m_stopSignalled = true;
    stopOverride();
}

TaskCompletionWatchObject Job::getCompletionWatchObject()
{
    return m_completed.get_future();
}

} // namespace meta
