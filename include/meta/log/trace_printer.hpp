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

#ifndef META_TRACE_PRINTER_HPP
#define META_TRACE_PRINTER_HPP

#include <meta/meta.hpp>

#include <memory>

namespace meta
{

struct TraceRecord;
/// Trace logger interface.
class META_API TracePrinter
{
public:
    /// Destructor.
    virtual ~TracePrinter() = default;

    virtual std::string format(const TraceRecord& trace) const = 0;

    /// Writes a trace record to the output.
    /// \param trace The trace record to write to the output.
    virtual void write(std::string text) = 0;
};
using TracePrinterPtr = std::shared_ptr<TracePrinter>;

/// Console output.
class META_API ConsoleOut : public TracePrinter
{
public:
    /// Constructor.
    explicit ConsoleOut() = default;

    /// Overrides TracePrinter::format().
    std::string format(const TraceRecord& trace) const override;

    /// Overide of TracePrinter::write(). Writes the text to the standard output.
    void write(std::string text) override;
};

} // namespace meta

#endif // META_TRACE_PRINTER_HPP
