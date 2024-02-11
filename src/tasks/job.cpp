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

#include <future>

namespace meta
{

struct Job::Descriptor
{
    // The worker task.
    std::packaged_task<void(Job*)> worker;
    // The job status.
    std::atomic<Job::Status> status = Job::Status::Deferred;

    static void main(Job* job)
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

    explicit Descriptor() :
        worker(&Descriptor::main)
    {
    }

    bool isNextStatusValid(Job::Status nextStatus)
    {
        auto currentStatus = status.load();
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
            default:
            {
                // Compiler yawn.
                return true;
            }
        }
    }
};


Job::Job() :
    descriptor(std::make_unique<Descriptor>())
{
}

Job::~Job()
{
    abortIfFail(descriptor->status == Status::Deferred || descriptor->status == Status::Stopped);
}

bool Job::canQueue() const
{
    return descriptor->isNextStatusValid(Status::Queued);
}

void Job::queue()
{
    descriptor->worker.reset();
    setStatus(Job::Status::Queued);
    onQueued();
}

void Job::schedule()
{
    if (!isStopped())
    {
        descriptor->worker(this);
    }
}

void Job::complete()
{
    if (descriptor->status.load() == Status::Completed)
    {
        setStatus(Job::Status::Deferred);
    }
    onCompleted();
}

Job::Status Job::getStatus() const
{
    return descriptor->status;
}

void Job::setStatus(Status status)
{
    abortIfFail(descriptor->isNextStatusValid(status));
    auto currentStatus = descriptor->status.load();
    // If the current status has changed, abort.
    abortIfFail(descriptor->status.compare_exchange_weak(currentStatus, status));
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

bool Job::isBusy() const
{
    const auto status = getStatus();
    return status == Status::Queued || status == Status::Running || status == Status::Completed;
}


void Job::wait()
{
    descriptor->worker.get_future().wait();
}

} // namespace meta
