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

#include <meta/threading.hpp>
#include <utility>

#if !defined(CONFIG_MULTI_THREADED)

namespace meta
{

void Mutex::lock()
{
    abortIfFail(m_lockCount == 0u);
    ++m_lockCount;
}

void Mutex::unlock()
{
    abortIfFail(m_lockCount > 0u);
    --m_lockCount;
}

bool Mutex::try_lock()
{
    if (m_lockCount != 0u)
    {
        return false;
    }
    lock();
    return true;
}


TaskPromise::TaskPromise() :
    m_data(std::make_shared<PromiseData>())
{
    m_data->syncValue = false;
}

TaskPromise::TaskPromise(TaskPromise&& other)
{
    swap(other);
}

TaskPromise& TaskPromise::operator=(TaskPromise&& other)
{
    TaskPromise tmp(std::forward<TaskPromise>(other));
    swap(tmp);
    return *this;
}

void TaskPromise::swap(TaskPromise& other)
{
    std::swap(other.m_data, m_data);
}

TaskFuture TaskPromise::get_future()
{
    TaskFuture future(m_data);
    return future;
}

void TaskPromise::set_value()
{
    abortIfFail(m_data);
    m_data->syncValue = true;
}


TaskFuture::TaskFuture(TaskPromise::PromiseDataPtr data) :
    m_data(data)
{
}

TaskFuture::TaskFuture(TaskFuture&& other)
{
    swap(other);
}

TaskFuture& TaskFuture::operator=(TaskFuture&& other)
{
    TaskFuture tmp(std::forward<TaskFuture>(other));
    swap(tmp);
    return *this;
}

void TaskFuture::swap(TaskFuture& other)
{
    std::swap(other.m_data, m_data);
}

void TaskFuture::wait()
{
    abortIfFail(m_data);
    while (!m_data->syncValue);
}


UniqueLock::UniqueLock(Mutex& mutex) :
    m_mutex(&mutex),
    m_owns(true)
{
    m_mutex->lock();
}
UniqueLock::UniqueLock(Mutex& mutex, defer_lock_t) :
    m_mutex(&mutex),
    m_owns(false)
{
}
UniqueLock::UniqueLock(Mutex& mutex, try_to_lock_t) :
    m_mutex(&mutex),
    m_owns(m_mutex->try_lock())
{
}
UniqueLock::UniqueLock(Mutex& mutex, adopt_lock_t) :
    m_mutex(&mutex),
    m_owns(true)
{
}
UniqueLock::~UniqueLock()
{
    if (m_owns)
    {
        m_mutex->unlock();
    }
}
void UniqueLock::swap(UniqueLock& other)
{
    std::swap(other.m_mutex, m_mutex);
    std::swap(other.m_owns, m_owns);
}
UniqueLock::UniqueLock(UniqueLock&& other) :
    m_mutex(std::move(other.m_mutex)),
    m_owns(std::move(other.m_owns))
{
    other.m_mutex = nullptr;
    other.m_owns = false;
}
UniqueLock& UniqueLock::operator=(UniqueLock&& other)
{
    if (m_owns)
    {
        m_mutex->unlock();
    }

    UniqueLock tmp(std::forward<UniqueLock>(other));
    swap(tmp);

    return *this;
}

void UniqueLock::lock()
{
    abortIfFail(m_mutex);
    abortIfFail(!m_owns);
    m_mutex->lock();
    m_owns = true;
}
void UniqueLock::unlock()
{
    abortIfFail(m_mutex);
    abortIfFail(m_owns);
    m_mutex->unlock();
    m_owns = false;
}

bool UniqueLock::try_lock()
{
    abortIfFail(m_mutex);
    abortIfFail(!m_owns);
    m_owns = m_mutex->try_lock();
    return m_owns;
}

bool UniqueLock::owns_lock() const
{
    return m_owns;
}
Mutex* UniqueLock::mutex() const
{
    return m_mutex;
}
Mutex* UniqueLock::release()
{
    Mutex* mutex = m_mutex;
    m_mutex = nullptr;
    m_owns = false;
    return mutex;
}


void ConditionVariable::notify_one()
{
}

void ConditionVariable::notify_all()
{
}

void ConditionVariable::wait(UniqueLock& lock)
{
    abortIfFail(lock.owns_lock());
    lock.unlock();
    lock.lock();
}


Thread::Thread(Thread&& other) :
    m_function(std::move(other.m_function)),
    m_joinable(std::move(other.m_joinable))
{
}

Thread& Thread::operator=(Thread&& other)
{
    Thread tmp(std::forward<Thread>(other));
    swap(tmp);
    return *this;
}

void Thread::swap(Thread& other)
{
    std::swap(other.m_function, m_function);
    std::swap(other.m_joinable, m_joinable);
}

void Thread::join()
{
    abortIfFail(m_joinable);
}

void Thread::detach()
{
    m_joinable = false;
}

}

#endif
