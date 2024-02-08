/*
 * Copyright (C) 2024 bitWelder
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

#ifndef META_TASK_SCHEDULER_PRIVATE_HPP
#define META_TASK_SCHEDULER_PRIVATE_HPP

#include <meta/tasks/job.hpp>
#include <meta/tasks/thread_pool.hpp>

#include <future>

namespace meta {

using GuardLock = std::lock_guard<std::mutex>;
using UniqueLock = std::unique_lock<std::mutex>;

namespace detail {

class JobPrivate
{
    friend class meta::Job;
    // The worker task.
    std::packaged_task<void(Job*)> worker;
    // The job status.
    std::atomic<Job::Status> status = Job::Status::Deferred;

    static void main(Job* job);

public:
    explicit JobPrivate();

    static void notifyJobQueued(Job& self);

    static bool isNextStatusValid(Job& self, Job::Status nextStatus);
    static void setStatus(Job& self, Job::Status nextStatus);

    static void runJob(JobPtr self);

    static void completeJob(JobPtr self);
};


}} // namespace meta::detail

#endif // META_TASK_SCHEDULER_PRIVATE_HPP
