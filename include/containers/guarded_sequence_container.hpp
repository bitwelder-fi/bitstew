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

#ifndef CONTAINERS_GUARDED_SEQUENCE_CONTAINER_HPP
#define CONTAINERS_GUARDED_SEQUENCE_CONTAINER_HPP

#include <utils/type_traits.hpp>
#include <utils/reference_counted.hpp>

#include <algorithm>
#include <optional>
#include <utility>

namespace containers
{

namespace detail
{

template <class ElementType>
bool isValid(const ElementType& value)
{
    return static_cast<bool>(value);
}


template <class ContainerType>
struct ForwardIteratorTraits
{
    static constexpr bool isConst = std::is_const_v<ContainerType>;
    using BaseIterator = std::conditional_t<isConst, typename ContainerType::const_iterator, typename ContainerType::iterator>;
    using Category = typename BaseIterator::iterator_category;
    using Pointer = std::conditional_t<isConst, typename ContainerType::const_pointer, typename ContainerType::pointer>;
    using Reference = std::conditional_t<isConst, typename ContainerType::const_reference, typename ContainerType::reference>;
};

template <class ContainerType>
struct ReverseIteratorTraits
{
    static constexpr bool isConst = std::is_const_v<ContainerType>;
    using BaseIterator = std::conditional_t<isConst, typename ContainerType::const_reverse_iterator, typename ContainerType::reverse_iterator>;
    using Category = typename BaseIterator::iterator_category;
    using Pointer = std::conditional_t<isConst, typename ContainerType::const_pointer, typename ContainerType::pointer>;
    using Reference = std::conditional_t<isConst, typename ContainerType::const_reference, typename ContainerType::reference>;
};



template <typename ContainerType, class Traits>
class IteratorProxy
{
    using SelfType = IteratorProxy<ContainerType, Traits>;

public:
    using BaseIterator      = Traits::BaseIterator;
    using iterator_category = Traits::Category;
    using value_type        = typename ContainerType::value_type;
    using difference_type   = typename ContainerType::difference_type;
    using pointer           = Traits::Pointer;
    using reference         = Traits::Reference;
    using size_type         = typename ContainerType::size_type;

    IteratorProxy(BaseIterator pos, BaseIterator end) :
        m_pos(pos),
        m_end(end)
    {
        while (m_pos != m_end && !isValid(*m_pos))
        {
            ++m_pos;
        }
    }

    IteratorProxy& operator++()
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

    IteratorProxy operator++(int)
    {
        IteratorProxy retval = *this;
        ++(*this);
        return retval;
    }

    bool operator==(IteratorProxy other) const
    {
        return other.m_pos == m_pos;
    }

    bool operator!=(IteratorProxy other) const
    {
        return !(*this == other);
    }

    reference operator*() const
    {
        return m_pos.operator*();
    }

    pointer operator->() const
    {
        return m_pos.operator->();
    }

    operator BaseIterator()
    {
        return m_pos;
    }

    friend size_type operator-(const SelfType& lhs, const SelfType& rhs)
    {
        return lhs.m_pos - rhs.m_pos;
    }

private:
    BaseIterator m_pos;
    BaseIterator m_end;
};

}

/// A guarded sequence container is a reference counted sequence container.
///
/// Use views to access the container elements. It is recommended to use guard locks to do that. These guards ensure that the
/// container gets locked before you try to access its content.
///
/// The container gets share-locked on read access, and exclusive-locked when you add or remove elements
/// from the container. Whilst read operations are only possible through views, write operations are
/// only possible through the interface of the container.
/// \tparam ElementType
template <class ContainerType>
class GuardedSequenceContainer : public utils::ReferenceCountLockable<GuardedSequenceContainer<ContainerType>>
{
    using SelfType = GuardedSequenceContainer<ContainerType>;
    friend class utils::ReferenceCountLockable<SelfType>;

    ContainerType m_container;

public:
    using value_type            = typename ContainerType::value_type;
    using reference             = typename ContainerType::reference;
    using const_reference       = typename ContainerType::const_reference;
    using size_type             = typename ContainerType::size_type;
    using difference_type       = typename ContainerType::difference_type;
    using pointer               = typename ContainerType::pointer;
    using const_pointer         = typename ContainerType::const_pointer;

    using Iterator = detail::IteratorProxy<
        ContainerType,
        detail::ForwardIteratorTraits<ContainerType>>;

    using ConstIterator = detail::IteratorProxy<
        const ContainerType,
        detail::ForwardIteratorTraits<const ContainerType>>;

    using ReverseIterator = detail::IteratorProxy<
        ContainerType,
        detail::ReverseIteratorTraits<ContainerType>>;

    using ConstReverseIterator = detail::IteratorProxy<
        const ContainerType,
        detail::ReverseIteratorTraits<const ContainerType>>;

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
        if (m_lockedView)
        {
            if (m_lockedView->inView(position))
            {
                return {};
            }
            auto result = m_container.insert(position, item);
            return Iterator(result, m_container.end());
        }

        return insert(position, item);
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
        if (m_lockedView)
        {
            if (m_lockedView->inView(position))
            {
                return {};
            }

            auto result = m_container.insert(position, std::forward<value_type>(item));
            return Iterator(result, m_container.end());
        }

        return insert(position, std::forward<value_type>(item));
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
        if (m_lockedView)
        {
            if (m_lockedView->inView(position))
            {
                *position = value_type();
                return Iterator(position, m_container.end());
            }

            // Erase outside of the view.
            m_container.erase(static_cast<typename ContainerType::iterator>(position));
            return {};
        }

        auto it = m_container.erase(static_cast<typename ContainerType::iterator>(position));
        return Iterator(it, m_container.end());
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
        if (m_lockedView)
        {
            if (m_lockedView->inView(position))
            {
                *position = value_type();
                return Iterator(position, m_container.end());
            }

            // Erase outside of the view.
            m_container.erase(static_cast<typename ContainerType::const_iterator>(position));
            return {};
        }

        auto it = m_container.erase(static_cast<typename ContainerType::const_iterator>(position));
        return Iterator(it, m_container.end());
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

    /// Method called by ReferenceCountLockable<> when the container gets locked the first time.
    View<Iterator> acquireResources()
    {
        if (!m_lockedView)
        {
            m_lockedView = std::move(std::make_optional(getView()));
        }
        return *m_lockedView;
    }

    /// Method called by ReferenceCountLockable<> when the container gets fully unlocked.
    void releaseResources()
    {
        auto predicate = [](auto& item) { return !detail::isValid(item); };
        m_container.erase(std::remove_if(m_container.begin(), m_container.end(), predicate), m_container.end());

        m_lockedView.reset();
    }
};

} // namespace utils

#endif // CONTAINERS_GUARDED_SEQUENCE_CONTAINER_HPP
