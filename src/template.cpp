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

#include <assert.hpp>
#include <pimpl.hpp>
#include <preprocessor.hpp>
#include <stew/utility/function_traits.hpp>
#include <stew/utility/scope_value.hpp>
#include <stew/utility/algorithm.hpp>

#include <stew/library_config.hpp>
#include <stew/stew.hpp>
#include <stew/metadata/factory.hpp>
#include <stew/object.hpp>
#include <stew/object_extensions/object_extension.hpp>
#include <stew/tasks/thread_pool.hpp>

#include <stew/log/trace.hpp>

#include <memory>

namespace stew
{

namespace
{

const std::string_view c_invalidMetaNameCharacters{"~`!@#$%^&+={[}]|\\;\"'<,>?/ "};

}

class MetaLibraryPrivate
{
public:
    DECLARE_PUBLIC_PTR(Library, MetaLibraryPrivate)

    explicit MetaLibraryPrivate()
    {
    }

    std::unique_ptr<ThreadPool> threadPool;
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
    if (arguments.threadPool.createThreadPool)
    {
        d->threadPool = std::make_unique<ThreadPool>(arguments.threadPool.threadCount);
        d->threadPool->start();
    }

#ifdef CONFIG_ENABLE_LOGS
    d->tracer = std::make_unique<Tracer>(d->threadPool.get());
    d->tracer->setLogLevel(arguments.tracer.logLevel);

    TracePrinterPtr printer = std::make_shared<ConsoleOut>();
    printer = std::make_shared<MessageSeparator>(printer);
    printer = std::make_shared<ThreadIdDecorator>(printer);
    // printer = std::make_shared<TimeStampDecorator>(printer);
    printer = std::make_shared<LogLevelDecorator>(printer);
    d->tracer->addTracePrinter(printer);
#endif

    d->objectFactory = std::make_unique<ObjectFactory>();
    d->objectFactory->registerMetaClass(Object::getStaticMetaClass());
    d->objectFactory->registerMetaClass(ObjectExtension::getStaticMetaClass());
}

void Library::uninitialize()
{
    D();

    if (d->threadPool && d->threadPool->isRunning())
    {
        d->threadPool->stop();
    }

    d->objectFactory.reset();
    if (d->tracer && d->tracer->isBusy())
    {
        d->tracer->stop();
        d->tracer->wait();
    }
    d->tracer.reset();
    d->threadPool.reset();
}

ThreadPool* Library::threadPool() const
{
    D();
    return d->threadPool.get();
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
    return !name.empty() && name.find_first_of(c_invalidMetaNameCharacters) == std::string_view::npos;
}

std::string ensureValidMetaName(std::string name, char hint)
{
    abortIfFail(!name.empty());

    auto hintIt = c_invalidMetaNameCharacters.find_first_of(hint);
    abortIfFail(hintIt == std::string_view::npos);

    std::size_t it = 0;
    while ( (it = name.find_first_of(c_invalidMetaNameCharacters)) != std::string::npos)
    {
        if (hint != 0)
        {
            name.at(it) = hint;
        }
        else
        {
            auto pos = name.begin() + it;
            name.erase(pos);
        }
    }
    abortIfFail(isValidMetaName(name));
    return name;
}


}
