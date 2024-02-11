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

#ifndef META_TRACE_HPP
#define META_TRACE_HPP

#include <meta/meta.hpp>
#include <meta/tasks/job.hpp>
#include <meta/log/trace_printer.hpp>
#include <meta/safe_queue.hpp>

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <sstream>
#include <thread>
#include <vector>

namespace meta
{

/// Defines the logging level of a trace line. Setting a logging level to the tracer implies that all
/// higher priority logs are also printed.
enum class LogLevel
{
    /// No logging on any levels.
    Suppressed,
    /// Identifies the fatal errors, from which the application casnnot recover.
    Fatal,
    /// Identifies the recoverable errors.
    Error,
    /// Identifies the warnings.
    Warning,
    /// Identifies the info traces.
    Info,
    /// Identifies the debug traces.
    Debug
};

/// The timestamp of a trace line.
using TimeStamp = std::chrono::time_point<std::chrono::system_clock>;

/// Declares the record of a trace line.
struct META_API TraceRecord
{
    /// The traced message.
    std::string message;
    /// The function in which the trace got logged.
    std::string function;
    /// The file in which the trace got logged.
    std::string file;
    /// The line at which the trace got logged.
    unsigned line;
    /// The time when the trace got logged.
    TimeStamp time;
    /// The thread where the trace got logged
    std::thread::id threadId;
    /// The logging level of the trace.
    LogLevel logLevel;

    /// Records a log now.
    explicit TraceRecord(LogLevel level, std::thread::id threadId, std::string_view function, std::string_view file, unsigned line, std::string_view message) :
        message(message),
        function(function),
        file(file),
        line(line),
        time(std::chrono::system_clock::now()),
        threadId(threadId),
        logLevel(level)
    {
    }
};

/// PrinterDecoratorInterface provides an interface to decorate a trace printer with elements of a
/// trace record.
class META_API PrinterDecoratorInterface
{
public:
    /// Destructor.
    virtual ~PrinterDecoratorInterface() = default;

    /// Returns the wrapped printer.
    virtual TracePrinterPtr getPrinter() const = 0;
 };

/// Trace manager.
class META_API Tracer : public Job
{
public:

    struct META_API Diagnostics
    {
        std::size_t bufferSize;
        std::size_t bufferOverflowCount;
    };
    /// Constructor.
    explicit Tracer(ThreadPool* threadPool);
    /// Destructor.
    ~Tracer();

    /// Sets the log level for the tracing.
    /// \param level The log level to set.
    void setLogLevel(LogLevel level)
    {
        m_logLevel = level;
    }

    /// Returns the tracing log level.
    /// \return The tracing log level.
    LogLevel getLogLevel() const
    {
        return m_logLevel;
    }

    /// Adds a trace output.
    void addTracePrinter(TracePrinterPtr output);

    /// Clears the outputs.
    void clearTracePrinters();

    /// Returns the printer as \a PrinterClass at \a index.
    template <class PrinterClass>
    std::shared_ptr<PrinterClass> getPrinterAt(std::size_t index)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        auto printer = m_outputs.at(index);
        while (printer)
        {
            auto wrappedType = std::dynamic_pointer_cast<PrinterClass>(printer);
            if (wrappedType)
            {
                return wrappedType;
            }

            auto decorator = std::dynamic_pointer_cast<PrinterDecoratorInterface>(printer);
            if (!decorator)
            {
                return {};
            }
            printer = decorator->getPrinter();
        }

        return {};
    }

    Diagnostics getDiagnostics() const
    {
        return {m_buffer.Capacity, m_bufferOverflowCount.load()};
    }

protected:
    void run() override;
    void onCompleted() override;

    TracePrinterPtr getTracePrinterAt(std::size_t index)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_outputs.at(index);
    }
    std::size_t getTracePrinterCount() const
    {
        return m_outputs.size();
    }

private:
    friend struct TracerPrivate;

    std::mutex m_mutex;
    std::vector<TracePrinterPtr> m_outputs;
    CircularBuffer<std::shared_ptr<const TraceRecord>, 10u> m_buffer;
    std::atomic_size_t m_bufferOverflowCount = 0u;
    ThreadPool* m_threadPool = nullptr;
    std::atomic<LogLevel> m_logLevel = LogLevel::Debug;
};
using TracerPtr = std::shared_ptr<Tracer>;

/// This class represents a log line.
class META_API LogLine : protected TraceRecord
{
public:
    explicit LogLine(LogLevel level, const char* function, const char* file, unsigned line);
    explicit LogLine(Tracer* tracer, LogLevel level, const char* function, const char* file, unsigned line);
    ~LogLine();

    std::ostream& operator()();

private:
    std::ostringstream m_printer;
    Tracer* m_tracer;
};

/// Extends a trace printer with nested formatters.
class META_API PrinterFormatter : public TracePrinter, public PrinterDecoratorInterface
{
    TracePrinterPtr m_printer;

public:
    /// Constructor.
    /// \param printer The trace printer this decorator extends.
    explicit PrinterFormatter(TracePrinterPtr printer) :
        m_printer(printer)
    {
    }
    /// Returns the trace printer this decorator extends.
    /// \return The trace printer this decorator extends.
    TracePrinterPtr getPrinter() const override
    {
        return m_printer;
    }

    /// Overrides final TracePrinter::write().
    void write(std::string text) final
    {
        m_printer->write(text);
    }
};

/// Log level formatter. Prepends the trace printer it extends with log level string.
class META_API LogLevelDecorator : public PrinterFormatter
{
public:
    /// Constructor.
    explicit LogLevelDecorator(TracePrinterPtr printer) :
        PrinterFormatter(printer)
    {
    }

    /// Overrides TraceFormatter::format();
    std::string format(const TraceRecord& trace) const override;
};

/// Thread id formatter. Prepends the trace printer it extends with stringified thread id.
class META_API ThreadIdDecorator : public PrinterFormatter
{
public:
    explicit ThreadIdDecorator(TracePrinterPtr printer) :
        PrinterFormatter(printer)
    {
    }
    /// Overrides TraceFormatter::format();
    std::string format(const TraceRecord& trace) const override;
};

/// Function name formatter. Prepends the trace printer it extends with the function name.
class META_API FunctionDecorator : public PrinterFormatter
{
public:
    explicit FunctionDecorator(TracePrinterPtr printer) :
        PrinterFormatter(printer)
    {
    }
    /// Overrides TraceFormatter::format();
    std::string format(const TraceRecord& trace) const override;
};

/// File line formatter. Prepends the trace printer it extends with file name and line.
class META_API FileLineDecorator : public PrinterFormatter
{
    std::string m_basePath;

public:
    explicit FileLineDecorator(TracePrinterPtr printer, std::string_view basePath);
    /// Overrides TraceFormatter::format();
    std::string format(const TraceRecord& trace) const override;
};

/// Adds a dash separator to the trace porinter it extends.
class META_API MessageSeparator : public PrinterFormatter
{
public:
    explicit MessageSeparator(TracePrinterPtr printer) :
        PrinterFormatter(printer)
    {
    }
    /// Overrides TraceFormatter::format();
    std::string format(const TraceRecord& trace) const override
    {
        return "- " + getPrinter()->format(trace);
    }
};

} // namespace meta

#ifdef CONFIG_ENABLE_LOGS

#define _META_CORE_TRACE(tracer, logLevel) \
meta::LogLine(tracer, logLevel, __FUNCTION__, __FILE__, __LINE__)()

#define _META_GLOBAL_TRACE(logLevel) \
meta::LogLine(logLevel, __FUNCTION__, __FILE__, __LINE__)()

#define META_LOG_FATAL(text) _META_GLOBAL_TRACE(meta::LogLevel::Fatal) << text
#define META_LOG_ERROR(text) _META_GLOBAL_TRACE(meta::LogLevel::Error) << text
#define META_LOG_WARNING(text) _META_GLOBAL_TRACE(meta::LogLevel::Warning) << text
#define META_LOG_INFO(text) _META_GLOBAL_TRACE(meta::LogLevel::Info) << text
#define META_LOG_DEBUG(text) _META_GLOBAL_TRACE(meta::LogLevel::Debug) << text
#else

#define META_LOG_FATAL(text)
#define META_LOG_ERROR(text)
#define META_LOG_WARNING(text)
#define META_LOG_INFO(text)
#define META_LOG_DEBUG(text)

#endif

#endif // META_TRACE_HPP
