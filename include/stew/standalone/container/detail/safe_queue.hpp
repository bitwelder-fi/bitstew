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

#ifndef STEW_DETAIL_SAFE_QUEUE_HPP
#define STEW_DETAIL_SAFE_QUEUE_HPP

#include <stew/stew_api.hpp>

#include <condition_variable>
#include <queue>

namespace stew { namespace detail {

template <class ElementType>
class STEW_TEMPLATE_API BaseSharedQueue
{
    std::queue<ElementType> m_buffer;

public:
    void push(ElementType data);
    ElementType pop();
    bool isEmpty() const;
};

template <typename ElementType>
class STEW_TEMPLATE_API SharedQueueNotifier
{
    using QueueType = BaseSharedQueue<ElementType>;
    QueueType& m_queue;
    std::condition_variable m_signal;

public:
    SharedQueueNotifier(QueueType& queue);
    void notifyOne();
    void wait(std::unique_lock<std::mutex> &lock);
};


// ----- Implementation -----
template <typename ElementType>
void BaseSharedQueue<ElementType>::push(ElementType data)
{
    m_buffer.push(std::move(data));
}

template <typename ElementType>
ElementType BaseSharedQueue<ElementType>::pop()
{
    if (m_buffer.empty())
    {
        return {};
    }
    auto data = m_buffer.front();
    m_buffer.pop();
    return data;
}

template <typename ElementType>
bool BaseSharedQueue<ElementType>::isEmpty() const
{
    return m_buffer.empty();
}

template <typename ElementType>
SharedQueueNotifier<ElementType>::SharedQueueNotifier(QueueType& queue) :
    m_queue(queue)
{
}

template <typename ElementType>
void SharedQueueNotifier<ElementType>::notifyOne()
{
    m_signal.notify_one();
}

template <typename ElementType>
void SharedQueueNotifier<ElementType>::wait(std::unique_lock<std::mutex> &lock)
{
    auto condition = [this]()
    {
        return !m_queue.nolock_isEmpty();
    };
    m_signal.wait(lock, condition);
}

}} // stew::detail

#endif
