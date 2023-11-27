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

#ifndef META_TRACE_OUTPUT_HPP
#define META_TRACE_OUTPUT_HPP

#include <meta/meta.hpp>

#include <string_view>

namespace meta {namespace log
{

/// Trace logger interface.
class META_API TraceOutput
{
public:
    /// Destructor.
    virtual ~TraceOutput() = default;
    /// Writes a text to the output.
    /// \param text The text to write to the output.
    virtual void write(std::string_view text) = 0;
};

/// Console output.
class META_API ConsoleOut : public TraceOutput
{
public:
    /// Constructor.
    explicit ConsoleOut() = default;

    /// Overide of TraceOutput::write(). Writes the text to the standard output.
    void write(std::string_view text) override;
};

}} // namespace meta::log

#endif // META_TRACE_OUTPUT_HPP
