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

#ifndef UTILS_THREAD_WRAPPER_HPP
#define UTILS_THREAD_WRAPPER_HPP


#include <meta/meta_api.hpp>

#if defined(CONFIG_MULTI_THREADED)

#include <atomic>
#include <condition_variable>
#include <future>
#include <thread>
#include <mutex>

#else

#include <assert.hpp>
#include <pimpl.hpp>

#include <future>

#endif

namespace meta
{

#if defined(CONFIG_MULTI_THREADED)

using ConditionVariable = std::condition_variable;
using Mutex = std::mutex;
using Thread = std::thread;
using ThreadId = std::thread::id;

template <class Callable>
using PackagedTask = std::packaged_task<Callable>;

template <class Result>
using Future = std::future<Result>;

using JobFuture = std::future<void>;

using UniqueLock = std::unique_lock<std::mutex>;
using GuardLock = std::lock_guard<std::mutex>;

template <typename T>
using Atomic = std::atomic<T>;
using AtomicBool = std::atomic_bool;

namespace ThisThread
{
using namespace std::this_thread;
}

#else

class META_API Mutex
{
    unsigned int m_lockCount = 0u;
public:
    explicit Mutex() = default;
    ~Mutex() = default;

    inline void lock()
    {
        abortIfFail(m_lockCount == 0u);
        ++m_lockCount;
    }
    inline void unlock()
    {
        abortIfFail(m_lockCount > 0u);
        --m_lockCount;
    }
    inline bool try_lock()
    {
        if (m_lockCount != 0u)
        {
            return false;
        }
        lock();
        return true;
    }
};

template <class T>
struct META_TEMPLATE_API SyncElement
{
    T syncValue;
};

struct META_API defer_lock_t { explicit defer_lock_t() = default; };
struct META_API try_to_lock_t { explicit try_to_lock_t() = default; };
struct META_API adopt_lock_t { explicit adopt_lock_t() = default; };

class META_API UniqueLock
{
    Mutex* m_mutex = nullptr;
    bool m_owns = false;
public:
    explicit UniqueLock() = default;
    explicit UniqueLock(Mutex& mutex) :
        m_mutex(&mutex),
        m_owns(true)
    {
        m_mutex->lock();
    }
    explicit UniqueLock(Mutex& mutex, defer_lock_t) :
        m_mutex(&mutex),
        m_owns(false)
    {
    }
    explicit UniqueLock(Mutex& mutex, try_to_lock_t) :
        m_mutex(&mutex),
        m_owns(m_mutex->try_lock())
    {
    }
    explicit UniqueLock(Mutex& mutex, adopt_lock_t) :
        m_mutex(&mutex),
        m_owns(true)
    {
    }
    ~UniqueLock()
    {
        if (m_owns)
        {
            m_mutex->unlock();
        }
    }
    inline void swap(UniqueLock& other)
    {
        std::swap(other.m_mutex, m_mutex);
        std::swap(other.m_owns, m_owns);
    }
    UniqueLock(UniqueLock&& other) :
        m_mutex(std::move(other.m_mutex)),
        m_owns(std::move(other.m_owns))
    {
        other.m_mutex = nullptr;
        other.m_owns = false;
    }
    inline UniqueLock& operator=(UniqueLock&& other)
    {
        if (m_owns)
        {
            m_mutex->unlock();
        }

        UniqueLock tmp(std::forward<UniqueLock>(other));
        swap(tmp);

        return *this;
    }
    DISABLE_COPY(UniqueLock);

    inline void lock()
    {
        abortIfFail(m_mutex);
        abortIfFail(!m_owns);
        m_mutex->lock();
        m_owns = true;
    }
    inline void unlock()
    {
        abortIfFail(m_mutex);
        abortIfFail(m_owns);
        m_mutex->unlock();
        m_owns = false;
    }
    inline bool try_lock()
    {
        abortIfFail(m_mutex);
        abortIfFail(!m_owns);
        m_owns = m_mutex->try_lock();
        return m_owns;
    }

    inline bool owns_lock() const
    {
        return m_owns;
    }
    inline Mutex* mutex() const
    {
        return m_mutex;
    }
    inline Mutex* release()
    {
        Mutex* mutex = m_mutex;
        m_mutex = nullptr;
        m_owns = false;
        return mutex;
    }
};

using GuardLock = UniqueLock;

class META_API ConditionVariable
{
public:
    explicit ConditionVariable() = default;
    DISABLE_COPY(ConditionVariable);

    inline void notify_one()
    {
    }
    inline void notify_all()
    {
    }

    inline void wait(UniqueLock& lock)
    {
        abortIfFail(lock.owns_lock());
        lock.unlock();
        lock.lock();
    }

    template <class Predicate>
    void wait(UniqueLock& lock, Predicate predicate)
    {
        while (!predicate())
        {
            wait(lock);
        }
    }
};

template <class T>
struct META_TEMPLATE_API Atomic
{
    explicit Atomic() = default;
    Atomic(T value) :
        m_value(value)
    {
    }
    DISABLE_COPY(Atomic);

    void store(T value)
    {
        m_value = value;
    }
    T load() const
    {
        return m_value;
    }

    T operator=(T value)
    {
        store(value);
        return value;
    }

    operator T() const
    {
        return load();
    }

    Atomic& operator++()
    {
        ++m_value;
        return *this;
    }

    Atomic& operator--()
    {
        --m_value;
        return *this;
    }

private:
    T m_value;
};

using AtomicBool = Atomic<bool>;

using JobFuture = std::future<void>;

template <class Callable>
using PackagedTask = std::packaged_task<Callable>;

using ThreadId = unsigned int;

class META_API Thread
{
public:
    explicit Thread() = default;
    ~Thread() = default;

    template <typename Function, class... Args>
    explicit Thread(Function&& f, Args&&... args)
    {
        auto lambda = [function = std::move(f), arguments = std::move(args...)] ()
        {
            function(arguments);
        };
        m_function = lambda;
    }

    Thread(Thread&& other) :
        m_function(std::move(other.m_function)),
        m_joinable(std::move(other.m_joinable))
    {
    }
    inline Thread& operator=(Thread&& other)
    {
        Thread tmp(std::forward<Thread>(other));
        swap(tmp);
        return *this;
    }
    inline void swap(Thread& other)
    {
        std::swap(other.m_function, m_function);
        std::swap(other.m_joinable, m_joinable);
    }
    DISABLE_COPY(Thread);

    inline bool joinable() const
    {
        return m_joinable;
    }
    inline void join()
    {
        abortIfFail(m_joinable);
    }
    inline void detach()
    {
        m_joinable = false;
    }
    inline ThreadId get_id() const
    {
        return 0u;
    }

    static unsigned hardware_concurrency()
    {
        return 0;
    }

private:
    friend class ThreadPool;
    std::function<void()> m_function;
    bool m_joinable = true;
};

namespace ThisThread
{

inline ThreadId get_id()
{
    return 0u;
}

}

#endif

}

#endif // UTILS_THREAD_WRAPPER_HPP
