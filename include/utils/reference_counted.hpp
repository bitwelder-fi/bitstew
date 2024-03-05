/*
 * Copyright (C) 2017-2024 bitWelder
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

#ifndef UTILS_REFERENCE_COUNTED_HPP
#define UTILS_REFERENCE_COUNTED_HPP

#include <assert.hpp>

#include <atomic>
#include <cstddef>

namespace utils
{

/// The reference counted object adds reference counting to an object. To use the feature, derive your
/// class from this template, and expose a cleanup() method, which gets invoked every time the reference
/// count reaches zero.
/// \tparam DerivedClass The class derived from this guarded object. By contract, the class must have
///         the following methods:
///         - getLockState() to get or create a locked snapshot of the object.
///         - releaseLockState() to release the losked snapshot of the object.
///
template <typename DerivedClass>
class ReferenceCounted
{
    DerivedClass* getDerived()
    {
        return static_cast<DerivedClass*>(this);
    }

public:
    /// Constructor.
    explicit ReferenceCounted() = default;

    /// Returns if the reference count is greater than zero.
    bool isLocked() const
    {
        return getLockCount() > 0u;
    }

    /// Returns the lock count of the vector.
    std::size_t getLockCount() const
    {
        return m_lockCount.load();
    }

    /// Locks the vector. Increments the lock count.
    auto lock()
    {
        auto newCount = m_lockCount.load() + 1;
        while (!m_lockCount.compare_exchange_weak(newCount, newCount + 1));

        return getDerived()->getLockState();
    }

    /// Unlocks the vector. Decrements the lock count. When the lock count reaches zero, calls the
    /// compact() method of the derived class.
    ///
    /// The call aborts if the guarded vector is not locked.
    void unlock()
    {
        abortIfFail(m_lockCount.load() > 0u);
        auto newCount = m_lockCount.load() - 1;
        while (!m_lockCount.compare_exchange_weak(newCount, newCount - 1));
        if (m_lockCount.load() == 0u)
        {
            static_cast<DerivedClass*>(this)->releaseLockState();
        }
    }

private:
    std::atomic_size_t m_lockCount = {0u};
};

} // namespace utils

#endif // UTILS_REFERENCE_COUNTED_HPP
