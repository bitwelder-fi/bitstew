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

#ifndef STEW_REFERENCE_COUNTED_HPP
#define STEW_REFERENCE_COUNTED_HPP

#include <stew/core/assert.hpp>

#include <atomic>
#include <cstddef>
#include <mutex>

namespace utils
{

/// The reference count lockable object adds reference counting to an object guarded with a mutex. To use
/// the feature, derive your class from this template, and expose the methods below.
/// \tparam DerivedClass The class derived from this guarded object. By contract, the class must have
///         the following methods:
///         - acquireResources() to acquire the resources of the object.
///         - releaseResources() to release the resources of the object.
template <class DerivedClass>
class ReferenceCountLockable
{
    DerivedClass* getDerived()
    {
        return static_cast<DerivedClass*>(this);
    }

public:
    using MutexType = std::mutex;

    /// Constructor.
    explicit ReferenceCountLockable() = default;

    /// Returns the lock count of the object.
    std::size_t getRefCount() const
    {
        return m_lockCount.load();
    }

    /// Retains the object and acquires its resources. Increments the reference count. Does not lock
    /// the mutex of the object.
    ///
    /// \return The return type depends on the derived class, whether that returns anything or it is
    ///         a void method.
    auto retain()
    {
        auto newCount = m_lockCount.load() + 1;
        while (!m_lockCount.compare_exchange_weak(newCount, newCount + 1));

        return getDerived()->acquireResources();
    }

    /// Releases an incerement of the object. Decrements the lock count of the object. When the lock
    /// count reaches zero, it releases the object resources invoking releaseResources() method. Does
    /// not unlock the mutex of the object.
    ///
    /// The call aborts if the guarded object is not locked.
    void release()
    {
        abortIfFail(m_lockCount.load() > 0u);

        auto newCount = m_lockCount.load() - 1;
        while (!m_lockCount.compare_exchange_weak(newCount, newCount - 1));
        if (m_lockCount.load() == 0u)
        {
            getDerived()->releaseResources();
        }
    }

    /// Returns the reference to the mutex of the reference counted object.
    MutexType& mutex()
    {
        return m_lock;
    }

private:
    mutable MutexType m_lock;
    std::atomic_size_t m_lockCount = {0u};
};

/// Unlocks an already locked reference count lockable object, and re-locks on destruction.
template <class LockableObject>
struct RelockGuard
{
    explicit RelockGuard(LockableObject& lockable) :
        m_mutex(lockable.mutex())
    {
        m_mutex.unlock();
    }
    ~RelockGuard()
    {
        m_mutex.lock();
    }

private:
    typename LockableObject::MutexType& m_mutex;
};

namespace detail
{

template <class LockableObject>
struct LockWrap
{
    explicit LockWrap(LockableObject* lockable) :
        mutex(lockable ? &lockable->mutex() : nullptr)
    {
    }

    void lock()
    {
        if (mutex)
        {
            mutex->lock();
        }
    }

    bool try_lock()
    {
        if (mutex)
        {
            return mutex->try_lock();
        }
        return true;
    }

    void unlock()
    {
        if (mutex)
        {
            mutex->unlock();
        }
    }

private:
    typename LockableObject::MutexType* mutex = nullptr;
};

}

template <class LockableObject>
struct ScopeLock
{
    explicit ScopeLock(LockableObject* lockable1, LockableObject* lockable2) :
        l1(lockable1), l2(lockable2),
        guard(l1, l2)
    {
    }

private:
    using MutexType = detail::LockWrap<LockableObject>;
    MutexType l1;
    MutexType l2;
    std::scoped_lock<MutexType, MutexType> guard;
};

template <class LockableObject>
struct LockGuard : public std::lock_guard<typename LockableObject::MutexType>
{
    explicit LockGuard(LockableObject& lockable) :
        std::lock_guard<typename LockableObject::MutexType>(lockable.mutex())
    {
    }
};

} // namespace utils

#endif // UTILS_REFERENCE_COUNTED_HPP
