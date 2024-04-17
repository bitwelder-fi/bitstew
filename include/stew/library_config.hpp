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

#ifndef STEW_LIBRARY_CONFIG_HPP
#define STEW_LIBRARY_CONFIG_HPP

#include <stew/stew_api.hpp>
#include <stew/log/trace.hpp>

namespace stew
{

/// The arguments with which the library gets initialized.
struct STEW_API LibraryArguments
{
    /// The task scheduler arguments.
    struct STEW_API ThreadPool
    {
        std::size_t threadCount = std::thread::hardware_concurrency();
        bool createThreadPool = true;
    } threadPool;

    struct STEW_API Tracer
    {
        LogLevel logLevel = LogLevel::Debug;
    } tracer;

    explicit LibraryArguments() = default;
};

}

#endif
