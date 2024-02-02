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

#include <meta/tasks/worker.hpp>
#include <meta/tasks/thread_pool.hpp>

namespace meta { namespace detail {

class TaskPrivate
{
public:
    static void notifyTaskQueued(Task& self, TaskScheduler* pool);

    static void notifyTaskScheduled(Task& self);

    static void runTask(Task& self);

    static bool isTaskQueued(Task& self);
};


}} // namespace meta::detail

#endif // META_TASK_SCHEDULER_PRIVATE_HPP
