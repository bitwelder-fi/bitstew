# Stew
A library of everything. 

This is a fresh remake of MOX, with different concepts, based on elements from C++20. And a bit of all kinds of extras, utilities.

*under construction*

[TOC]

# The story

Stew was started as a library for meta-programming, and to support dynamic typing. And then it slowly got moved towards a toolkit, and became an amalgam of all sorts of utility modules, and it is intended to grow in that direction. The repo structure is organized so that it is easy to extract modules by their header, and source. Standalone, pure header modules are located under `include/stew/standalone` folder, which may only rely on the core of stew, located in `include/stew/core`. Each module is described by a separate read.me file.

The core components of **stew** library are grouped in a singleton, the [Library](./include/stew/stew.hpp), which you must initialize at your application startup, and respectively uninitialize on application tear down.

```cpp
#include <stew/stew.hpp>

int main(...)
{
    // Initialize the stew library with the default configuration:
    // - thread count set to the maximum thread capability of the HW
    // - log level set to Debug.
    stew::Library::instance().initialize();

    // Your code comes here.

    // Tear down meta library.
    stew::Library::instance().uninitialize();
}
```

# Library components

## The core

The core consists of a set of header files defining preprocessor, PIMPL, assertion and API exposure macros for stew.

## The standalone

The folder contains header-only tools you can reuse in your projects for various purposes. Type traits, function traits, caches, guarded containers, etc. The majority of these depend on some of the core headers.

### Caches

The sub-module contains various caches. Read more [here](./include/stew/standalone/cache/README.md).

### Containers

These are guarded containers, with invalid element-safe iterators and range views. Read more [here](./include/stew/standalone/container/README.md).

### Utilities

You can read more about utilities [here](./include/stew/standalone/utility/README.md). 

## Arguments

The arguments is the core of the dynamic typing. *Under construction.* Read more [here](./include/stew/arguments/README.md).

## Thread pool

`std::async()` documentation states that its implementation may or may not be pooled. **meta** comes with a thread pool implementation, which queues jobs whose state you can observe during the execution time of the job. You can create [Job](./include/stew/tasks/job.hpp)s which can re-schedule themselves on completion, or create jobs with queues, where you can also use one of the safe queues provided by the library.

To run a job with thread pool, use `meta::async()` or using the thread pool of the library.

You can configure the number of threads of the pool at the library initialization phase.

## Logging

You can enable tracing at build time, by turning `CONFIG_ENABLE_LOGS` build flag on. The log level gets configured at the library initialization time. The [Tracer](./include/stew/log/trace.hpp) is a self-rescheduling job of the **Meta** library, which the logging system component uses to print logs.

You can install multiple log printers to **Meta**, and each log printer can have a chain of log decorators. **Meta** provides the following printers and decorators:

- [TracePrinter](./include/stew/log/trace_printer.hpp#TracePrinter) - the log printer interface.
- [ConsoleOut](./include/stew/log/trace_printer.hpp#ConsoleOut) - the log printer to print to `std::cout`.

and the following decorators:

- [PrinterDecoratorInterface](./include/stew/log/trace.hpp#PrinterDecoratorInterface) - the printer decorator interface.
- [LogLevelDecorator](./include/stew/log/trace.hpp#LogLevelDecorator) - decorates the log with readable log level.
- [ThreadIdDecorator](./include/stew/log/trace.hpp#ThreadIdDecorator) - decorates the log with the thread ID.
- [FunctionDecorator](./include/stew/log/trace.hpp#FunctionDecorator) - decorates the log with function name.
- [FileLineDecorator](./include/stew/log/trace.hpp#FileLineDecorator) - decorates the log with file and line number.
- [MessageDecorator](./include/stew/log/trace.hpp#MessageDecorator) - decorates the log with the log message.

To trace logs, use the dedicated macros for the log level:

- `STEW_LOG_FATAL(text)` to log fatal errors.
- `STEW_LOG_ERROR(text)` to log recoverable errors.
- `STEW_LOG_WARNING(text)` to log warnings.
- `STEW_LOG_INFO(text)` to log info traces.
- `STEW_LOG_DEBUG(text)` to log debug traces.

## Meta

Meta (dynamic) programming sub-module provides a bridge for scripting, as well as dynamic programming. Read more [here](./include/stew/meta/README.md).

## Scripting
*Future, consider LUA*

# What's next

- Property types, meta properties
- Property bindigs, binding expressions
