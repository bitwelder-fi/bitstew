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

#include <chrono>
#include <functional>
#include <memory>

#endif

namespace meta
{

#if defined(CONFIG_MULTI_THREADED)

using ConditionVariable = std::condition_variable;
using Mutex = std::mutex;
using Thread = std::thread;
using ThreadId = std::thread::id;
using TaskCompletionSignal = std::promise<void>;
using TaskCompletionWatchObject = std::future<void>;

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

    void lock();
    void unlock();
    bool try_lock();
};

template <class T>
struct META_TEMPLATE_API SyncElement
{
    T syncValue;
};

class TaskCompletionWatchObject;
class META_API TaskCompletionSignal
{
public:
    using PromiseData = SyncElement<bool>;
    using PromiseDataPtr = std::shared_ptr<PromiseData>;

    explicit TaskCompletionSignal();
    ~TaskCompletionSignal() = default;
    TaskCompletionSignal(TaskCompletionSignal&& other);
    TaskCompletionSignal& operator=(TaskCompletionSignal&& other);
    void swap(TaskCompletionSignal& other);
    DISABLE_COPY(TaskCompletionSignal);

    TaskCompletionWatchObject get_future();

    void set_value();

private:
    PromiseDataPtr m_data;
};

class META_API TaskCompletionWatchObject
{
    friend class TaskCompletionSignal;

public:
    explicit TaskCompletionWatchObject() = default;
    ~TaskCompletionWatchObject() = default;
    TaskCompletionWatchObject(TaskCompletionWatchObject&& other);
    TaskCompletionWatchObject& operator=(TaskCompletionWatchObject&& other);
    void swap(TaskCompletionWatchObject& other);
    DISABLE_COPY(TaskCompletionWatchObject);

    void wait();
    bool valid() const;

private:
    explicit TaskCompletionWatchObject(TaskCompletionSignal::PromiseDataPtr data);

    TaskCompletionSignal::PromiseDataPtr m_data;
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
    explicit UniqueLock(Mutex& mutex);
    explicit UniqueLock(Mutex& mutex, defer_lock_t);
    explicit UniqueLock(Mutex& mutex, try_to_lock_t);
    explicit UniqueLock(Mutex& mutex, adopt_lock_t);
    ~UniqueLock();
    void swap(UniqueLock& other);
    UniqueLock(UniqueLock&& other);
    UniqueLock& operator=(UniqueLock&& other);
    DISABLE_COPY(UniqueLock);

    void lock();
    void unlock();
    bool try_lock();

    bool owns_lock() const;
    Mutex* mutex() const;
    Mutex* release();
};

using GuardLock = UniqueLock;

class META_API ConditionVariable
{
public:
    explicit ConditionVariable() = default;
    DISABLE_COPY(ConditionVariable);

    void notify_one();
    void notify_all();

    void wait(UniqueLock& lock);

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
struct META_API Atomic
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

    Thread(Thread&& other);
    Thread& operator=(Thread&& other);
    void swap(Thread& other);
    DISABLE_COPY(Thread);

    bool joinable() const _NOEXCEPT
    {
        return m_joinable;
    }
    void join();
    void detach();
    ThreadId get_id() const _NOEXCEPT
    {
        return 0u;
    }

    static unsigned hardware_concurrency() _NOEXCEPT
    {
        return 0;
    }

private:
    friend class TaskScheduler;
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
