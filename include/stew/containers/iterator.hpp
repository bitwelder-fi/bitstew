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

#ifndef STEW_ITERATOR_HPP
#define STEW_ITERATOR_HPP

#include <stew/utility/type_traits.hpp>

namespace containers
{

namespace
{

template <class N>
concept non_floating_point = !std::floating_point<N>;

}

/// An iterator which wraps an iterator type of a container. The iterator ensures that the element of
/// the container it points to is always valid, unless it points to the end iterator.
template <class Container, class BaseIterator, class Pointer, class Reference>
class IteratorWrap
{
    using SelfType = IteratorWrap<Container, BaseIterator, Pointer, Reference>;

public:
    using iterator_category = typename BaseIterator::iterator_category;
    using value_type        = typename BaseIterator::value_type;
    using difference_type   = typename BaseIterator::difference_type;
    using pointer           = Pointer;
    using reference         = Reference;
    using size_type         = typename Container::size_type;

    /// Default constructor.
    IteratorWrap() = default;

    /// Constructor. Initializes the iterator for a given position. Adjusts the iterator to point to
    /// the first valid element of the container, between the pos and end.
    /// \param pos The position of the iterator.
    /// \param end The end position of the iterator.
    IteratorWrap(Container& container, BaseIterator pos, BaseIterator end) :
        m_container(&container),
        m_pos(pos),
        m_end(end)
    {
        while (m_pos != m_end && !m_container->isValid(*m_pos))
        {
            ++m_pos;
        }
    }

    /// Increment operator.
    IteratorWrap& operator++()
    {
        if (m_pos == m_end)
        {
            throw std::out_of_range("out of view range");
        }
        while (++m_pos != m_end)
        {
            if (m_container->isValid(*m_pos))
            {
                return *this;
            }
        }

        return *this;
    }

    /// Left-increment operator.
    IteratorWrap operator++(int)
    {
        IteratorWrap retval = *this;
        ++(*this);
        return retval;
    }

    /// Decrement operator.
    IteratorWrap& operator--()
    {
        while (!m_container->isValid(*(--m_pos)));

        return *this;
    }

    /// Left-decrement operator.
    IteratorWrap operator--(int)
    {
        IteratorWrap retval = *this;
        --(*this);
        return retval;
    }

    /// Advancing operator.
    IteratorWrap& operator+=(difference_type distance)
    {
        if (distance > 0)
        {
            while (distance-- > 0)
            {
                ++(*this);
            }
        }
        return *this;
    }

    /// Equality comparation operator.
    bool operator==(IteratorWrap other) const
    {
        return other.m_pos == m_pos;
    }

    /// Differs operator.
    bool operator!=(IteratorWrap other) const
    {
        return !(*this == other);
    }

    /// Deref operator.
    reference operator*() const
    {
        return m_pos.operator*();
    }

    /// Pointer access operator.
    pointer operator->() const
    {
        return m_pos.operator->();
    }

    /// Cast operator, casts to the base iterator type.
    operator BaseIterator()
    {
        return m_pos;
    }

    /// Difference operator.
    friend size_type operator-(const SelfType& last, const SelfType& first)
    {
        size_type diff = 0;
        for (auto it = first; it != last; ++it)
        {
            ++diff;
        }
        return diff;
    }

    friend bool operator <(IteratorWrap lhs, IteratorWrap rhs)
    {
        return lhs.m_pos < rhs.m_pos;
    }
    friend bool operator <=(IteratorWrap lhs, IteratorWrap rhs)
    {
        return lhs.m_pos <= rhs.m_pos;
    }
    friend bool operator >(IteratorWrap lhs, IteratorWrap rhs)
    {
        return lhs.m_pos > rhs.m_pos;
    }
    friend bool operator >=(IteratorWrap lhs, IteratorWrap rhs)
    {
        return lhs.m_pos >= rhs.m_pos;
    }

private:
    Container* m_container = nullptr;
    BaseIterator m_pos;
    BaseIterator m_end;
};

/// Increment operator, returns the iterator pointing to an element at \a distance from the position.
/// \param position The iterator at position.
/// \param distance The distance to which to move the position.
/// \return The iterator pointing to the position at distance.
template <class Iterator>
Iterator operator+(Iterator position, typename Iterator::size_type distance)
{
    position += distance;
    return position;
}

}

#endif // STEW_ITERATOR_HPP
