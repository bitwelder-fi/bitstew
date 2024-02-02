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

#ifndef META_TASK_SCHEDULER_PRIVATE_HPP
#define META_TASK_SCHEDULER_PRIVATE_HPP

#include <meta/tasks/job.hpp>
#include <meta/tasks/thread_pool.hpp>

namespace meta { namespace detail {

class JobPrivate
{
public:
    static void notifyJobQueued(Job& self, ThreadPool* pool);

    static void notifyJobScheduled(Job& self, ThreadId threadId);

    static void runJob(Job& self);

    static bool isTaskQueued(Job& self);
};


}} // namespace meta::detail

#endif // META_TASK_SCHEDULER_PRIVATE_HPP
