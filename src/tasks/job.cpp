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

void JobPrivate::notifyJobQueued(Job& self, ThreadPool*)
{
    self.m_worker.reset();
    self.setStatus(Job::Status::Queued);
    self.onTaskQueued();
}

void JobPrivate::notifyJobScheduled(Job& self, ThreadId threadId)
{
    self.setStatus(Job::Status::Scheduled);
    self.m_owningThread = threadId;
    self.onJobScheduled();
}

bool JobPrivate::isTaskQueued(Job& self)
{
    return self.m_status != Job::Status::Stopped || self.m_status != Job::Status::Stopped;
}

void JobPrivate::runJob(Job& self)
{
    self.m_worker(self);
}

} // namespace detail


Job::Job() :
    m_worker([](Job& self)
    {
        abortIfFail(self.m_status == Job::Status::Scheduled);

        self.setStatus(Job::Status::Running);

        if (!self.m_stopSignalled)
        {
            self.run();
        }

        self.setStatus(Job::Status::Stopped);

        self.onTaskCompleted();
    })
{
}

Job::~Job()
{
    abortIfFail(m_status == Status::Deferred || m_status == Status::Stopped);
}

void Job::reset()
{
    abortIfFail(m_status == Status::Deferred || m_status == Status::Stopped);

    m_worker.reset();
    m_stopSignalled = false;
    m_owningThread = ThreadId();
    setStatus(Status::Deferred);
}

void Job::stop()
{
    m_stopSignalled = true;
    stopOverride();
}

JobFuture Job::getFuture()
{
    return m_worker.get_future();
}

} // namespace meta
