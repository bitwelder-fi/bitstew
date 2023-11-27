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
#include <meta/task.hpp>
#include <utils/thread_wrapper.hpp>

#include <memory>
#include <queue>
#include <vector>

namespace meta { namespace log {

class TraceOutput;
using TraceOutputPtr = std::unique_ptr<TraceOutput>;

/// Trace manager.
class META_API Tracer final : public meta::Task
{
public:
    /// Constructor.
    explicit Tracer() = default;
    /// Destructor.
    ~Tracer();

    /// Adds a trace output.
    void addOutput(TraceOutputPtr output);

    /// Clears the outputs.
    void clearOutputs();

    /// Logs the text to the outputs.
    void log(std::string_view text);

protected:
    void runOverride();
    void runOnce();

private:
    utils::Mutex m_mutex;
    utils::ConditionVariable m_signal;
    std::vector<TraceOutputPtr> m_outputs;
    std::queue<std::string> m_buffer;
};

}} // namespace meta::log

#endif // META_TRACE_HPP
