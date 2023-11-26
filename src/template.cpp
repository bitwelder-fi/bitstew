/*
 * Copyright (C) 2017-2019 bitWelder
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
#include <pimpl.hpp>
#include <preprocessor.hpp>
#include <utils/function_traits.hpp>
#include <utils/lockable.hpp>
#include <utils/scope_value.hpp>
#include <utils/utility.hpp>
#include <utils/vector.hpp>

#include <meta/library_config.hpp>
#include <meta/meta.hpp>
#include <meta/tasks/task_scheduler.hpp>

#include <memory>

namespace meta
{

class MetaLibraryPrivate
{
public:
    DECLARE_PUBLIC_PTR(Domain, MetaLibraryPrivate)

    explicit MetaLibraryPrivate()
    {
    }

    std::unique_ptr<TaskScheduler> taskScheduler;
};

Domain& Domain::instance()
{
    static Domain meta;
    return meta;
}

Domain::Domain() :
    d_ptr(pimpl::make_d_ptr<MetaLibraryPrivate>())
{
}

Domain::~Domain()
{
    uninitialize();
}

void Domain::initialize(const LibraryArguments& arguments)
{
    D();
    if (arguments.taskScheduler.createThreadPool)
    {
        d->taskScheduler = std::make_unique<TaskScheduler>(arguments.taskScheduler.threadCount);
    }
}

void Domain::uninitialize()
{
    D();
    if (d->taskScheduler)
    {
        if (d->taskScheduler->isRunning())
        {
            d->taskScheduler->stop();
        }
        d->taskScheduler.reset();
    }
}

TaskScheduler* Domain::taskScheduler() const
{
    D();
    return d->taskScheduler.get();
}

}
