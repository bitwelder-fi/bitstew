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

#include <containers/iterator.hpp>
#include <containers/view.hpp>
#include <utils/type_traits.hpp>
#include <utils/reference_counted.hpp>

#include <algorithm>
#include <optional>
#include <utility>

namespace containers
{

/// A guarded sequence container is a reference counted sequence container.
///
/// Use views to access the container elements. It is recommended to use guard locks to do that. These guards ensure that the
/// container gets locked before you try to access its content.
///
/// The container gets share-locked on read access, and exclusive-locked when you add or remove elements
/// from the container. Whilst read operations are only possible through views, write operations are
/// only possible through the interface of the container.
/// \tparam ElementType
template <class ContainerType, bool (*ValidElementFunction)(const typename ContainerType::value_type&)>
class GuardedSequenceContainer : public utils::ReferenceCountLockable<GuardedSequenceContainer<ContainerType, ValidElementFunction>>
{
    using SelfType = GuardedSequenceContainer<ContainerType, ValidElementFunction>;
    friend class utils::ReferenceCountLockable<SelfType>;

    ContainerType m_container;

    /// Forward iterator traits.
    template <class TContainer>
    struct ForwardIteratorTraits
    {
        static constexpr bool isConst = std::is_const_v<TContainer>;

        using BaseIterator = std::conditional_t<isConst, typename ContainerType::const_iterator, typename ContainerType::iterator>;
        using Pointer = std::conditional_t<isConst, typename TContainer::const_pointer, typename TContainer::pointer>;
        using Reference = std::conditional_t<isConst, typename TContainer::const_reference, typename TContainer::reference>;
        using SizeType = typename TContainer::size_type;

        bool (*valid_element)(const typename ContainerType::value_type&) = ValidElementFunction;
    };

    /// Reverse iterator traits.
    template <class TContainer>
    struct ReverseIteratorTraits
    {
        static constexpr bool isConst = std::is_const_v<TContainer>;

        using BaseIterator = std::conditional_t<isConst, typename ContainerType::const_reverse_iterator, typename ContainerType::reverse_iterator>;
        using Pointer = std::conditional_t<isConst, typename TContainer::const_pointer, typename TContainer::pointer>;
        using Reference = std::conditional_t<isConst, typename TContainer::const_reference, typename TContainer::reference>;
        using SizeType = typename TContainer::size_type;

        bool (*valid_element)(const typename ContainerType::value_type&) = ValidElementFunction;
    };

public:
    using Container         = ContainerType;
    using value_type        = typename ContainerType::value_type;
    using reference         = typename ContainerType::reference;
    using const_reference   = typename ContainerType::const_reference;
    using size_type         = typename ContainerType::size_type;
    using difference_type   = typename ContainerType::difference_type;
    using pointer           = typename ContainerType::pointer;
    using const_pointer     = typename ContainerType::const_pointer;

    bool (*valid_element)(const typename ContainerType::value_type&) = ValidElementFunction;

    /// Creates a guarded vector container.
    explicit GuardedSequenceContainer() = default;

    /// \name View
    /// \{

    /// The forward iterator of the guarded container.
    using Iterator = IteratorWrap<ForwardIteratorTraits<SelfType>>;

    /// The forward const iterator of the guarded container.
    using ConstIterator = IteratorWrap<ForwardIteratorTraits<const SelfType>>;

    /// The reverse iterator of the guarded container.
    using ReverseIterator = IteratorWrap<ReverseIteratorTraits<SelfType>>;

    /// The reverse const iterator of the guarded container.
    using ConstReverseIterator = IteratorWrap<ReverseIteratorTraits<const SelfType>>;

    Iterator begin()
    {
        return Iterator(m_container.begin(), m_container.end());
    }
    ConstIterator begin() const
    {
        return ConstIterator(m_container.begin(), m_container.end());
    }
    ConstIterator cbegin() const
    {
        return ConstIterator(m_container.cbegin(), m_container.cend());
    }
    ReverseIterator rbegin()
    {
        return ReverseIterator(m_container.rbegin(), m_container.rend());
    }
    ConstReverseIterator rbegin() const
    {
        return ConstReverseIterator(m_container.rbegin(), m_container.rend());
    }
    ConstReverseIterator crbegin() const
    {
        return ConstReverseIterator(m_container.crbegin(), m_container.crend());
    }
    Iterator end()
    {
        return Iterator(m_container.end(), m_container.end());
    }
    ConstIterator end() const
    {
        return ConstIterator(m_container.end(), m_container.end());
    }
    ConstIterator cend() const
    {
        return ConstIterator(m_container.cend(), m_container.cend());
    }
    ReverseIterator rend()
    {
        return ReverseIterator(m_container.rend(), m_container.rend());
    }
    ConstReverseIterator rend() const
    {
        return ConstReverseIterator(m_container.rend(), m_container.rend());
    }
    ConstReverseIterator crend() const
    {
        return ConstReverseIterator(m_container.crend(), m_container.crend());
    }

    /// The locked view type of the container.
    using LockedViewType = View<ContainerType, ConstIterator>;
    using LockedView = std::optional<LockedViewType>;

    /// Returns the locked view of the container.
    LockedView getLockedView() const
    {
        return m_lockedView;
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
                auto dist = std::distance(cbegin(), position);
                auto pos = begin();
                std::advance(pos, dist);
                *pos = value_type();
                return Iterator(pos, m_container.end());
            }

            // Erase outside of the view.
            m_container.erase(position);
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
    LockedViewType acquireResources()
    {
        if (!m_lockedView)
        {
            m_lockedView = std::move(std::make_optional(LockedViewType(cbegin(), cend())));
        }
        return *m_lockedView;
    }

    /// Method called by ReferenceCountLockable<> when the container gets fully unlocked.
    void releaseResources()
    {
        auto predicate = [this](auto& item) { return !valid_element(item); };
        m_container.erase(std::remove_if(m_container.begin(), m_container.end(), predicate), m_container.end());

        m_lockedView.reset();
    }
};

} // namespace utils

#endif // CONTAINERS_GUARDED_SEQUENCE_CONTAINER_HPP
