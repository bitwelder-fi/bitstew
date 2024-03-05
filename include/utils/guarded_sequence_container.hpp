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

#ifndef UTILS_GUARDED_SEQUENCE_CONTAINER_HPP
#define UTILS_GUARDED_SEQUENCE_CONTAINER_HPP

#include <utils/type_traits.hpp>
#include <utils/reference_counted.hpp>

#include <algorithm>
#include <optional>
#include <utility>

namespace utils
{

namespace detail
{

template <class ElementType>
bool isValid(const ElementType& value)
{
    return static_cast<bool>(value);
}

template <typename ContainerType, typename BaseIterator, typename Category, typename Pointer, typename Reference>
class IteratorWrap
{
    BaseIterator m_pos;
    BaseIterator m_end;

public:
    using iterator_category = Category;
    using value_type        = typename ContainerType::value_type;
    using difference_type   = typename ContainerType::difference_type;
    using pointer           = Pointer;
    using reference         = Reference;

    IteratorWrap(BaseIterator pos, BaseIterator end) :
        m_pos(pos),
        m_end(end)
    {
        while (m_pos != m_end && !isValid(*m_pos))
        {
            ++m_pos;
        }
    }

    IteratorWrap& operator++()
    {
        while (++m_pos != m_end)
        {
            if (isValid(*m_pos))
            {
                break;
            }
        }

        return *this;
    }

    IteratorWrap operator++(int)
    {
        IteratorWrap retval = *this;
        ++(*this);
        return retval;
    }

    bool operator==(IteratorWrap other) const
    {
        return other.m_pos == m_pos;
    }

    bool operator!=(IteratorWrap other) const
    {
        return !(*this == other);
    }

    Reference operator*() const
    {
        return m_pos.operator*();
    }

    Pointer operator->() const
    {
        return m_pos.operator->();
    }

    operator BaseIterator()
    {
        return m_pos;
    }
};

}

/// A guarded deque is a deque guarded through a reference counted lock.
/// \tparam ElementType
template <class ContainerType>
class GuardedSequenceContainer : public ReferenceCounted<GuardedSequenceContainer<ContainerType>>
{
    using SelfType = GuardedSequenceContainer<ContainerType>;
    friend class ReferenceCounted<SelfType>;

    ContainerType m_container;

public:
    using value_type            = typename ContainerType::value_type;
    using reference             = typename ContainerType::reference;
    using const_reference       = typename ContainerType::const_reference;
    using size_type             = typename ContainerType::size_type;
    using difference_type       = typename ContainerType::difference_type;
    using pointer               = typename ContainerType::pointer;
    using const_pointer         = typename ContainerType::const_pointer;

    using Iterator = detail::IteratorWrap<
        ContainerType,
        typename ContainerType::iterator,
        typename ContainerType::iterator::iterator_category,
        pointer,
        reference>;

    using ConstIterator = detail::IteratorWrap<
        ContainerType,
        typename ContainerType::const_iterator,
        typename ContainerType::const_iterator::iterator_category,
        const_pointer,
        const_reference>;

    using ReverseIterator = detail::IteratorWrap<
        ContainerType,
        typename ContainerType::reverse_iterator,
        typename ContainerType::reverse_iterator::iterator_category,
        pointer,
        reference>;

    using ConstReverseIterator = detail::IteratorWrap<
        ContainerType,
        typename ContainerType::const_reverse_iterator,
        typename ContainerType::const_reverse_iterator::iterator_category,
        const_pointer,
        const_reference>;

    /// A view of iterators of the container.
    template <class IteratorType>
    struct View
    {
        explicit View(SelfType& self) :
            m_viewBegin(IteratorType(self.m_container.begin(), self.m_container.end())),
            m_viewEnd(IteratorType(self.m_container.end(), self.m_container.end()))
        {
        }
        explicit View(const SelfType& self) :
            m_viewBegin(IteratorType(self.m_container.begin(), self.m_container.end())),
            m_viewEnd(IteratorType(self.m_container.end(), self.m_container.end()))
        {
        }

        /// \name Iterators
        /// \{
        IteratorType begin() const
        {
            return m_viewBegin;
        }
        IteratorType end() const
        {
            return m_viewEnd;
        }
        bool inView(IteratorType position) const
        {
            for (auto it = m_viewBegin; it != m_viewEnd; ++it)
            {
                if (it == position)
                {
                    return true;
                }
            }
            return false;
        }
        /// \}

        /// name Capacity
        /// \{
        bool isEmpty() const
        {
            return size() > 0u;
        }
        size_type size() const
        {
            return std::distance(m_viewBegin, m_viewEnd);
        }
        /// \}

        /// \name Element access in a view.
        /// \{

        std::optional<IteratorType> find(const value_type& item)
        {
            auto pos = std::find(m_viewBegin, m_viewEnd, item);
            if (pos != m_viewEnd)
            {
                return IteratorType(pos, m_viewEnd);
            }

            return {};
        }
        /// \}

    private:
        IteratorType m_viewBegin;
        IteratorType m_viewEnd;
    };

    /// The locked view type of the container.
    using LockedView = std::optional<View<Iterator>>;

    /// Creates a guarded vector container.
    explicit GuardedSequenceContainer() = default;

    /// name Capacity
    /// \{
    bool isEmpty() const
    {
        return m_container.empty();
    }
    size_type size() const
    {
        return m_container.size();
    }
    /// \}

    /// \name Iterators
    /// \{

    /// Returns the locked view of the container.
    LockedView getLockedView() const
    {
        return m_lockedView;
    }

    /// Returns an unlocked view of the container.
    View<Iterator> getView()
    {
        return View<Iterator>(*this);
    }

    /// Returns an unlocked view of the container with const iterators.
    View<ConstIterator> getConstView() const
    {
        return View<ConstIterator>(*this);
    }

    /// Returns an unlocked view of the container with reverse iterators.
    View<ReverseIterator> getReverseView()
    {
        return View<ReverseIterator>(*this);
    }

    /// Returns an unlocked view of the container with const reverse iterators.
    View<ConstReverseIterator> getConstReverseView() const
    {
        return View<ConstReverseIterator>(*this);
    }
    /// \}

    /// \name Element access
    /// \{
    reference at(size_type index)
    {
        return m_container.at(index);
    }
    const_reference at(size_type index) const
    {
        return m_container.at(index);
    }
    reference operator[](size_type index)
    {
        return m_container[index];
    }
    const_reference operator[](size_type index) const
    {
        return m_container[index];
    }
    /// \}

    /// \name Modifiers
    /// \{

    /// Clears the container. If the container is locked, it resets the elements of the container.
    /// The container will get cleared once it gets fully unlocked.
    void clear()
    {
        if (this->isLocked())
        {
            std::for_each(m_lockedView->begin(), m_lockedView->end(), [](auto& item) { item = value_type(); });
        }
        else
        {
            m_container.clear();
        }
    }

    /// Inserts a copy of an item at position. The operation fails if the insert position is inside
    /// the locked view of the container.
    ///
    /// \param position The position where to insert the item.
    /// \param item The item to insert.
    /// \return On success, returns the iterator pointing to the inserted item. On failure returns a
    ///         nullopt.
    std::optional<Iterator> insert(Iterator position, const value_type& item)
    {
        if (m_lockedView && m_lockedView->inView(position))
        {
            return {};
        }

        auto result = m_container.insert(position, item);
        return Iterator(result, m_container.end());
    }

    /// Inserts an item at position. The operation fails if the insert position is inside
    /// the locked view of the container.
    ///
    /// \param position The position where to insert the item.
    /// \param item The item to insert.
    /// \return On success, returns the iterator pointing to the inserted item. On failure returns a
    ///         std::nullopt.
    std::optional<Iterator> insert(Iterator position, value_type&& item)
    {
        if (m_lockedView && m_lockedView->inView(position))
        {
            return {};
        }

        auto result = m_container.insert(position, std::forward<value_type>(item));
        return Iterator(result, m_container.end());
    }

    /// Erases or resets the item at position.
    /// - The element is reset if the position is within the locked view of the container.
    /// - The element is removed if the position is outside of the locked view.
    ///
    /// \param position The position of the element to erase.
    /// \return The iterator which follows the erased element. If the container is locked, and the
    ///         position falls outside of the locked view, returns std::nullopt.
    std::optional<Iterator> erase(Iterator position)
    {
        if (m_lockedView && m_lockedView->inView(position))
        {
            *position = value_type();
            return Iterator(position, m_container.end());
        }
        else
        {
            auto it = m_container.erase(static_cast<ContainerType::iterator>(position));

            if (this->isLocked())
            {
                return {};
            }
            return Iterator(it, m_container.end());
        }
    }

    /// Erases or resets the item at position.
    /// - The element is reset if the position is within the locked view of the container.
    /// - The element is removed if the position is outside of the locked view.
    ///
    /// \param position The position of the element to erase.
    /// \return The iterator which follows the erased element. If the container is locked, and the
    ///         position falls outside of the locked view, returns std::nullopt.
    std::optional<Iterator> erase(ConstIterator position)
    {
        if (m_lockedView && m_lockedView->inView(position))
        {
            *position = value_type();
            return Iterator(position, m_container.end());
        }
        else
        {
            auto it = m_container.erase(position);

            if (this->isLocked())
            {
                return {};
            }
            return Iterator(it, m_container.end());
        }
    }

    /// Adds an element at the end of the container.
    /// \param element The element to add to the end of the container.
    void push_back(const value_type& element)
    {
        m_container.push_back(element);
    }

    /// Moves an element to the end of the container.
    /// \param element The element to move to the end of the container.
    void push_back(value_type&& element)
    {
        m_container.push_back(std::forward<value_type>(element));
    }
    /// \}

private:
    LockedView m_lockedView;
    /// Method called by ReferenceCounted<> when the container gets locked the first time.
    View<Iterator> getLockState()
    {
        if (!m_lockedView)
        {
            m_lockedView = std::move(std::make_optional(getView()));
        }
        return *m_lockedView;
    }

    /// Method called by ReferenceCounted<> when the container gets fully unlocked.
    void releaseLockState()
    {
        auto predicate = [](auto& item) { return !detail::isValid(item); };
        m_container.erase(std::remove_if(m_container.begin(), m_container.end(), predicate), m_container.end());

        m_lockedView.reset();
    }
};

} // namespace utils

#endif // UTILS_GUARDED_SEQUENCE_CONTAINER_HPP
