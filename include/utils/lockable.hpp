/*
 * Copyright (C) 2017-2019 bitWelder
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

#ifndef LOCKABLE_HPP
#define LOCKABLE_HPP

#include <config.hpp>

namespace utils
{

#ifdef PLATFORM_CONFIG_THREAD_ENABLED

#include <mutex>
using mutex_type = std::mutex;

#else

struct mutex_type
{
    explicit mutex_type() = default;
    ~mutex_type() = default;

    void lock()
    {
        abortIfFail(m_lockCount == 0u);
        ++m_lockCount;
    }

    void unlock()
    {
        abortIfFail(m_lockCount == 1u);
        --m_lockCount;
    }

    bool try_lock()
    {
        if (m_lockCount == 1u)
        {
            return false;
        }
        ++m_lockCount;
        return true;
    }

private:
    DISABLE_COPY(mutex_type)
    DISABLE_MOVE(mutex_type)

    size_t m_lockCount = 0u;
};

#endif

} // namespace utils

#endif // LOCKABLE_HPP
