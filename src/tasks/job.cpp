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
    if (job->isStopped())
    {
        return;
    }

    job->setStatus(Job::Status::Running);
    job->run();
    if (job->getStatus() != Job::Status::Stopped)
    {
        job->setStatus(Job::Status::Completed);
    }
}

void JobPrivate::notifyJobQueued(Job& self)
{
    self.descriptor->worker.reset();
    self.setStatus(Job::Status::Queued);
    self.onQueued();
}

bool JobPrivate::isNextStatusValid(Job& self, Job::Status nextStatus)
{
    auto currentStatus = self.descriptor->status.load();
    switch (currentStatus)
    {
        case Job::Status::Deferred:
        {
            return (nextStatus == Job::Status::Deferred || nextStatus == Job::Status::Queued || nextStatus == Job::Status::Stopped);
        }
        case Job::Status::Queued:
        {
            return (nextStatus == Job::Status::Running || nextStatus == Job::Status::Stopped);
        }
        case Job::Status::Running:
        {
            return (nextStatus == Job::Status::Completed || nextStatus == Job::Status::Stopped);
        }
        case Job::Status::Completed:
        {
            return (nextStatus == Job::Status::Deferred || nextStatus == Job::Status::Stopped);
        }
        case Job::Status::Stopped:
        {
            return (nextStatus == Job::Status::Stopped);
        }
    }
}

void JobPrivate::setStatus(Job& self, Job::Status nextStatus)
{
    abortIfFail(isNextStatusValid(self, nextStatus));
    auto currentStatus = self.descriptor->status.load();
    // If the current status has changed, abort.
    abortIfFail(self.descriptor->status.compare_exchange_weak(currentStatus, nextStatus));
}

void JobPrivate::runJob(JobPtr self)
{
    if (self && !self->isStopped())
    {
        self->descriptor->worker(self.get());
    }
}

void JobPrivate::completeJob(JobPtr self)
{
    if (!self)
    {
        return;
    }
    if (self->descriptor->status.load() == Job::Status::Completed)
    {
        setStatus(*self, Job::Status::Deferred);
    }
    self->onCompleted();
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
    detail::JobPrivate::setStatus(*this, status);
}

void Job::stop()
{
    setStatus(Status::Stopped);
    stopOverride();
}

bool Job::isStopped() const
{
    return getStatus() == Status::Stopped;
}


void Job::wait()
{
    descriptor->worker.get_future().wait();
}

} // namespace meta
