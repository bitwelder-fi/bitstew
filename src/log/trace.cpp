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
#include <meta/log/trace.hpp>
#include <meta/log/trace_printer.hpp>
#include <meta/tasks/task_scheduler.hpp>
#include <meta/threading.hpp>

namespace meta
{

struct TracerPrivate
{
    static void log(Tracer& self, const TraceRecord& trace)
    {
        if (static_cast<std::size_t>(trace.logLevel) > static_cast<std::size_t>(self.m_logLevel.load()))
        {
            return;
        }

        {
            GuardLock lock(self.m_mutex);
            self.m_buffer.push(trace);
        }
        self.m_signal.notify_one();

        if (self.m_taskScheduler)
        {
            const auto status = self.getStatus();
            if (status == Task::Status::Deferred || status == Task::Status::Stopped)
            {
                self.m_taskScheduler->tryQueueTask(self.shared_from_this());
            }
            // self.m_taskScheduler->schedule();
            return;
        }

        // The thread pool is not active, run the task.
        self.setStatus(Task::Status::Scheduled);
        self.run();
    }

    static void print(Tracer& self, const TraceRecord& trace)
    {
        for (auto& out : self.m_outputs)
        {
            auto text = out->format(trace);
            out->write(text);
        }
    }
};


Tracer::Tracer(TaskScheduler* taskScheduler) :
    m_taskScheduler(taskScheduler)
{
}

Tracer::~Tracer()
{
}

// Consume the buffer when scheduled.
void Tracer::runOverride()
{
    UniqueLock lock(m_mutex);
    auto condition = [this]()
    {
        return !this->m_buffer.empty() || isStopped();
    };
    m_signal.wait(lock, condition);
    while (!m_buffer.empty())
    {
        auto trace = m_buffer.front();
        m_buffer.pop();
        TracerPrivate::print(*this, trace);
    }
}

void Tracer::stopOverride()
{
    m_signal.notify_all();
}

void Tracer::addTracePrinter(TracePrinterPtr output)
{
    GuardLock lock(m_mutex);
    m_outputs.push_back(output);
}

void Tracer::clearTracePrinters()
{
    GuardLock lock(m_mutex);
    m_outputs.clear();
}


LogLine::LogLine(Tracer* tracer, LogLevel level, const char* function, const char* file, unsigned line) :
    TraceRecord(level, ThisThread::get_id(), function, file, line, ""),
    m_tracer(tracer)

{
    abortIfFail(m_tracer);
}

LogLine::LogLine(LogLevel level, const char* function, const char* file, unsigned line) :
    LogLine(meta::Library::instance().tracer(), level, function, file, line)
{
}

LogLine::~LogLine()
{
    message = std::string(m_printer.str());
    TracerPrivate::log(*m_tracer, *this);
}

std::ostream& LogLine::operator()()
{
    return m_printer;
}


std::string  LogLevelDecorator::format(const TraceRecord& trace) const
{
    switch (trace.logLevel)
    {
        case LogLevel::Fatal:
        {
            return "[FATAL] " + getPrinter()->format(trace);
        }
        case LogLevel::Error:
        {
            return "[ERROR] " + getPrinter()->format(trace);
        }
        case LogLevel::Warning:
        {
            return "[WARNING] " + getPrinter()->format(trace);
        }
        case LogLevel::Info:
        {
            return "[INFO] " + getPrinter()->format(trace);
        }
        case LogLevel::Debug:
        {
            return "[DEBUG] " + getPrinter()->format(trace);
        }
        default:
        {
            return getPrinter()->format(trace);
        }
    }
}


std::string ThreadIdDecorator::format(const TraceRecord& trace) const
{
    std::ostringstream ss;
    ss << "tid[" << trace.threadId << "] " << getPrinter()->format(trace);
    return ss.str();
}


std::string FunctionDecorator::format(const TraceRecord& trace) const
{
    return trace.function + ' ' + getPrinter()->format(trace);
}


FileLineDecorator::FileLineDecorator(TracePrinterPtr printer, std::string_view basePath) :
    PrinterFormatter(printer),
    m_basePath(basePath)
{
}

std::string FileLineDecorator::format(const TraceRecord& trace) const
{
    std::string file(trace.file);
    if (!m_basePath.empty())
    {
        auto basePathPos = file.find(m_basePath);
        if (basePathPos != std::string::npos)
        {
            file.erase(basePathPos, m_basePath.length());
        }
    }

    std::ostringstream ss;
    ss << file << ":" << trace.line << ' ' << getPrinter()->format(trace);
    return ss.str();
}

} // namespace meta
