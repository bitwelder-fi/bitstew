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

#ifndef CONTAINERS_ITERATOR_HPP
#define CONTAINERS_ITERATOR_HPP

#include <utils/type_traits.hpp>


namespace containers
{

/// An iterator which wraps an iterator type of a container. The iterator ensures that the element of
/// the container it points to is always valid, unless it points to the end iterator.
template <class Traits>
class IteratorWrap
{
    using SelfType = IteratorWrap<Traits>;

public:
    using BaseIterator      = typename Traits::BaseIterator;
    using iterator_category = typename BaseIterator::iterator_category;
    using value_type        = typename BaseIterator::value_type;
    using difference_type   = typename BaseIterator::difference_type;
    using pointer           = typename Traits::Pointer;
    using reference         = typename Traits::Reference;
    using size_type         = typename Traits::SizeType;

    /// Default constructor.
    IteratorWrap() = default;

    /// Constructor. Initializes the iterator for a given position. Adjusts the iterator to point to
    /// the first valid element of the container, between the pos and end.
    /// \param pos The position of the iterator.
    /// \param end The end position of the iterator.
    IteratorWrap(BaseIterator pos, BaseIterator end, value_type invalidElement) :
        m_pos(pos),
        m_end(end),
        m_invalidElement(invalidElement)
    {
        while (m_pos != m_end && *m_pos == m_invalidElement)
        {
            ++m_pos;
        }
    }

    /// Increment operator.
    IteratorWrap& operator++()
    {
        while (++m_pos != m_end)
        {
            if (*m_pos != m_invalidElement)
            {
                break;
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
    friend size_type operator-(const SelfType& lhs, const SelfType& rhs)
    {
        return lhs.m_pos - rhs.m_pos;
    }

private:
    BaseIterator m_pos;
    BaseIterator m_end;
    value_type m_invalidElement = {};
};

}

#endif // CONTAINERS_ITERATOR_HPP
