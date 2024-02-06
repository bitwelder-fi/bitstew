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
#include <mutex>
#include <deque>

namespace meta
{

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
template <class ElementType>
class META_TEMPLATE_API SharedQueue
{
    std::deque<ElementType> m_buffer;
    std::mutex m_lock;

public:
    /// Pushes the element into the queue. The method returns when the element gets pushed with success.
    /// \param element The element to push into the queue.
    void push(ElementType element)
    {
        std::lock_guard<decltype(m_lock)> lock(m_lock);
        m_buffer.push_back(std::move(element));
    }

    /// Returns the element at head of the queue, and advances the head.
    /// \return The element at head on success, or an invalid element when the queue is empty.
    ElementType pop()
    {
        std::lock_guard<decltype(m_lock)> lock(m_lock);
        if (m_buffer.empty())
        {
            return {};
        }

        auto data = m_buffer.front();
        m_buffer.pop_front();
        return data;
    }

    /// Returns whether the queue is empty.
    /// \return If the queue is empty, returns \e true, otherwise \e false.
    bool isEmpty() const
    {
        return m_buffer.empty();
    }
};

}

#endif // META_SAFE_QUEUE_HPP
