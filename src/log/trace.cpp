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
#include <stew/log/trace.hpp>
#include <stew/log/trace_printer.hpp>
#include <stew/tasks/thread_pool.hpp>

#include <iostream>
#include <thread>

namespace stew
{

struct TracerPrivate
{
    static void log(Tracer& self, const TraceRecord& trace)
    {
        if (static_cast<std::size_t>(trace.logLevel) > static_cast<std::size_t>(self.m_logLevel.load()))
        {
            return;
        }

        auto data = std::make_shared<const TraceRecord>(trace);
        auto job = self.shared_from_this();
        auto successfulReschedule = false;

        while (!self.m_buffer.tryPush(data))
        {
            if (self.m_threadPool)
            {
                successfulReschedule |= self.m_threadPool->tryScheduleJob(job);
            }
            self.m_bufferOverflowCount++;
        }

        if (self.m_threadPool)
        {
            if (!successfulReschedule)
            {
                self.m_threadPool->tryScheduleJob(job);
            }
        }
        else
        {
            // The thread pool is not active, run the task.
            async(job);
        }
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


Tracer::Tracer(ThreadPool* threadPool) :
    m_threadPool(threadPool)
{
}

Tracer::~Tracer()
{
}

// Consume the buffer when scheduled.
void Tracer::run()
{
    for (auto data = m_buffer.tryPop(); data; data = m_buffer.tryPop())
    {
        TracerPrivate::print(*this, *data);
    }
}

void Tracer::onCompleted()
{
    if (m_buffer.wasEmpty())
    {
        return;
    }
    if (m_threadPool)
    {
        m_threadPool->tryScheduleJob(shared_from_this());
    }
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
    TraceRecord(level, std::this_thread::get_id(), function, file, line, ""),
    m_tracer(tracer)

{
    abortIfFail(m_tracer);
}

LogLine::LogLine(LogLevel level, const char* function, const char* file, unsigned line) :
    LogLine(stew::Library::instance().tracer(), level, function, file, line)
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

} // namespace stew
