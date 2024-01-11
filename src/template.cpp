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
#include <pimpl.hpp>
#include <preprocessor.hpp>
#include <utils/function_traits.hpp>
#include <utils/lockable.hpp>
#include <utils/scope_value.hpp>
#include <utils/utility.hpp>
#include <utils/vector.hpp>

#include <meta/library_config.hpp>
#include <meta/meta.hpp>
#include <meta/metadata/factory.hpp>
#include <meta/metadata/metaobject.hpp>
#include <meta/tasks/task_scheduler.hpp>

#include <meta/log/trace.hpp>

#include <memory>

namespace meta
{

class MetaLibraryPrivate
{
public:
    DECLARE_PUBLIC_PTR(Library, MetaLibraryPrivate)

    explicit MetaLibraryPrivate()
    {
    }

    std::unique_ptr<TaskScheduler> taskScheduler;
    std::shared_ptr<Tracer> tracer;
    std::unique_ptr<ObjectFactory> objectFactory;
};

Library& Library::instance()
{
    static Library meta;
    return meta;
}

Library::Library() :
    d_ptr(pimpl::make_d_ptr<MetaLibraryPrivate>())
{
}

Library::~Library()
{
    uninitialize();
}

void Library::initialize(const LibraryArguments& arguments)
{
    D();
    if (arguments.taskScheduler.createThreadPool)
    {
        d->taskScheduler = std::make_unique<TaskScheduler>(arguments.taskScheduler.threadCount);
        d->taskScheduler->start();
    }

#ifdef CONFIG_ENABLE_LOGS
    d->tracer = std::make_unique<Tracer>(d->taskScheduler.get());
    d->tracer->setLogLevel(arguments.tracer.logLevel);

    TracePrinterPtr printer = std::make_shared<ConsoleOut>();
    printer = std::make_shared<MessageSeparator>(printer);
    printer = std::make_shared<ThreadIdDecorator>(printer);
    // printer = std::make_shared<TimeStampDecorator>(printer);
    printer = std::make_shared<LogLevelDecorator>(printer);
    d->tracer->addTracePrinter(printer);
#endif

    d->objectFactory = std::make_unique<ObjectFactory>();
    d->objectFactory->registerMetaClass(MetaObject::getStaticMetaClass());
}

void Library::uninitialize()
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
    d->tracer.reset();
}

TaskScheduler* Library::taskScheduler() const
{
    D();
    return d->taskScheduler.get();
}

Tracer* Library::tracer() const
{
    D();
    return d->tracer.get();
}

ObjectFactory* Library::objectFactory() const
{
    D();
    return d->objectFactory.get();
}


bool isValidMetaName(std::string_view name)
{
    return name.find_first_of("~`!@#$%^&*()+={[}]|\\;\"'<,>?/ ") == std::string_view::npos;
}

}
