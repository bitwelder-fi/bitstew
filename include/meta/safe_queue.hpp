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

#ifndef META_SAFE_QUEUE_HPP
#define META_SAFE_QUEUE_HPP

#include <meta/meta_api.hpp>

#include <atomic>
#include <array>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <deque>

namespace meta
{

namespace queue
{

class META_API SharedQueueNotifier
{
public:
    template <typename Condition>
    void setCondition(Condition condition)
    {
        m_condition = condition;
    }

    void notifyOne()
    {
        m_signal.notify_one();
    }

    void wait(std::unique_lock<std::mutex> &lock)
    {
        m_signal.wait(lock, m_condition);
    }

protected:
    std::condition_variable m_signal;
    std::function<bool()> m_condition;
};

}

/// A lock-free thread safe circular buffer with elements of arbitrar type.
/// \tparam ElementType The element type.
/// \tparam Size The size of the circular buffer. Defaults to 4 elements.
template <class ElementType, std::size_t Size = 4>
class META_TEMPLATE_API CircularBuffer
{
    using Buffer = std::array<ElementType, Size>;
    Buffer buffer;

    std::atomic_size_t head = 0u;
    std::atomic_size_t tail = 0u;

    std::size_t increment(std::size_t index) const
    {
        return (index + 1u) % Size;
    }

public:
    /// The capacity of the circular buffer.
    const std::size_t Capacity = Size;

    /// Pushed an element into the circular buffer. On siccess, the element gets moved into the buffer.
    /// \param element The element to push into the buffer.
    /// \retusn On success, returns \e trus. The method fails if the buffer is full, in which case
    ///         returns \e false.
    bool push(ElementType element)
    {
        auto currentTail = tail.load();
        if (increment(currentTail) != head.load())
        {
            buffer[currentTail] = std::move(element);
            while(!tail.compare_exchange_weak(currentTail, increment(currentTail)));
            return true;
        }

        return false;
    }

    /// Returns the element at head of the curcular buffer, and advances the head.
    /// \return The element at head on success, or an invalid element when the curcular buffer is empty.
    ElementType pop()
    {
        auto currentHead = head.load();
        if (currentHead == tail.load())
        {
            // empty.
            return {};
        }

        auto item = std::move(buffer[currentHead]);
        while (!head.compare_exchange_weak(currentHead, increment(currentHead)));

        return item;
    }

    /// Returns whether the curcular buffer is empty.
    /// \return If the circular buffer is empty, returns \e true, otherwise \e false.
    bool isEmpty() const
    {
        return head.load() == tail.load();
    }
};

/// A shared, thread safe dynamic queue of elements. The queue gets locked on every push and pop call.
/// \tparam ElementType The type of an element of the shared queue.
/// \tparam Notifier The notifier of the shared queue. The notifier is expected to have the following
///         API:
///         -# The constructor should take a reference to the
///         -# auto notifyOne(): to notify a single change.
///         -# auto wait(stdL:unique_lock<std::mutex>&): to wait on a lock.
template <class ElementType, class Notifier = queue::SharedQueueNotifier>
class META_TEMPLATE_API SharedQueue
{
    std::deque<ElementType> m_buffer;
    std::mutex m_lock;
    Notifier m_notifier;

    ElementType _pop()
    {
        if (m_buffer.empty())
        {
            return {};
        }

        auto data = m_buffer.front();
        m_buffer.pop_front();
        return data;
    }

public:
    explicit SharedQueue()
    {
        if constexpr (std::is_same_v<Notifier, queue::SharedQueueNotifier>)
        {
            m_notifier.setCondition([this]() { return !m_buffer.empty(); });
        }
    }

    /// The notifier of the shared queue.
    Notifier& getNotifier()
    {
        return m_notifier;
    }
    /// Pushes the element into the queue. The method returns when the element gets pushed with success.
    /// \param element The element to push into the queue.
    void push(ElementType element)
    {
        {
            std::lock_guard<decltype(m_lock)> lock(m_lock);
            m_buffer.push_back(std::move(element));
        }
        m_notifier.notifyOne();
    }

    /// Waits till the queue gets notified, returns the element at the head of the queue, and advances
    /// the head. If the queue is empty, returns an invalid element.
    /// \return The element at head on success, or an invalid element when the queue is empty.
    ElementType pop()
    {
        std::unique_lock<std::mutex> lock(m_lock);
        m_notifier.wait(lock);

        return _pop();
    }

    /// Waits till the queue gets notified, and applies the command on each popped element of the queue.
    /// \tparam Command The command with syntax `bool(ElementType&)` to execute for each individual
    ///         element of the queue. The loop stops if the queue gets emptied or the command returns
    ///         false.
    /// \param command The command to execute on the elements of the queue.
    template <typename Command>
    void forEach(Command command)
    {
        std::unique_lock<std::mutex> lock(m_lock);
        m_notifier.wait(lock);
        while (!m_buffer.empty() && command(_pop()));
    }

    /// Returns whether the queue is empty.
    /// \return If the queue is empty, returns \e true, otherwise \e false.
    bool isEmpty()
    {
        std::lock_guard<std::mutex> lock(m_lock);
        return m_buffer.empty();
    }

    /// Returns a state of the queue, in a non-thread-safe manner. Use this method only when you are
    /// providing a custom notifier.
    /// \return If the queue is empty, returns \e true, otherwise \e false.
    bool unsafe_isEmpty() const
    {
        return m_buffer.empty();
    }
};

}

#endif // META_SAFE_QUEUE_HPP
