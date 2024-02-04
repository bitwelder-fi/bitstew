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

JobPrivate::JobPrivate() :
    worker(&JobPrivate::main)
{
}

void JobPrivate::main(Job* job)
{
    abortIfFail(job && job->descriptor->status == Job::Status::Scheduled);

    job->setStatus(Job::Status::Running);

    if (!job->descriptor->stopSignalled)
    {
        job->run();
    }

    job->setStatus(Job::Status::Stopped);

    job->onTaskCompleted();
}

void JobPrivate::notifyJobQueued(Job& self)
{
    self.descriptor->worker.reset();
    self.setStatus(Job::Status::Queued);
    self.onTaskQueued();
}

void JobPrivate::notifyJobScheduled(Job& self)
{
    self.setStatus(Job::Status::Scheduled);
    self.onJobScheduled();
}

void JobPrivate::runJob(Job& self)
{
    self.descriptor->worker(&self);
}

} // namespace detail


Job::Job() :
    descriptor(std::make_unique<detail::JobPrivate>())
{
}

Job::~Job()
{
    abortIfFail(descriptor->status == Status::Deferred || descriptor->status == Status::Stopped);
}

Job::Status Job::getStatus() const
{
    return descriptor->status;
}

void Job::setStatus(Status status)
{
    descriptor->status = status;
}

void Job::reset()
{
    abortIfFail(descriptor->status == Status::Deferred || descriptor->status == Status::Stopped);

    descriptor->worker.reset();
    descriptor->stopSignalled = false;
    setStatus(Status::Deferred);
}

void Job::stop()
{
    descriptor->stopSignalled = true;
    stopOverride();
}

bool Job::isStopped() const
{
    return descriptor->stopSignalled;
}


void Job::wait()
{
    descriptor->worker.get_future().wait();
}

} // namespace meta
