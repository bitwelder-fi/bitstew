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

#ifndef META_TTL_CLOCK_HPP
#define META_TTL_CLOCK_HPP

#include <meta/meta_api.hpp>

#include <chrono>

namespace meta
{

/// The clock object which provides the Time To Leave values for a cache.
class META_API TtlClock
{
public:
    using clock         = std::chrono::steady_clock;
    using duration      = clock::duration;
    using time_point    = clock::time_point;

    /// Returns the duration value of the given milliseconds.
    /// \param ms The milliseconds for which to provide the duration.
    /// \return The duration.
    inline static auto msecs(std::size_t ms)
    {
        return std::chrono::milliseconds(ms);
    }

    /// Returns the current time.
    /// \return The current time.
    inline static auto now()
    {
        return clock::now();
    }
};

}

#endif // META_TTL_CLOCK_HPP
