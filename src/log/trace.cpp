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

#include <meta/log/trace.hpp>
#include <meta/log/trace_output.hpp>
#include <meta/thread_pool/thread_pool.hpp>

namespace meta { namespace log {

Tracer::~Tracer()
{
}

void Tracer::runOnce()
{
    utils::UniqueLock lock(m_mutex);
    auto condition = [this]()
    {
        return !this->m_buffer.empty() || isStopped();
    };
    m_signal.wait(lock, condition);
    while (!m_buffer.empty())
    {
        auto text = m_buffer.front();
        m_buffer.pop();
        for (auto& out : m_outputs)
        {
            out->write(text);
        }
    }
}

// Consume the buffer when scheduled.
void Tracer::runOverride()
{
    if (m_threadPool)
    {
        while (!isStopped())
        {
            runOnce();
        }
    }
    else
    {
        runOnce();
    }
}

void Tracer::addOutput(TraceOutputPtr output)
{
    utils::GuardLock lock(m_mutex);
    m_outputs.push_back(std::move(output));
}

void Tracer::clearOutputs()
{
    utils::GuardLock lock(m_mutex);
    m_outputs.clear();
}

void Tracer::log(std::string_view text)
{
    auto threadPool = meta::Domain::instance().threadPool();
    if (threadPool)
    {
        {
            utils::GuardLock lock(m_mutex);
            m_buffer.push(std::string(text));
        }
        // Push this task into the pool.
        threadPool->addTask(shared_from_this());
        // Notify tracer.
        m_signal.notify_one();
        return;
    }

    // The thread pool is not active, run the task.
    setStatus(Status::Queued);
    run();
}

}} // namespace meta::log
