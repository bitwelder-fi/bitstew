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

template <class T>
concept guardable_sequence_container = !traits::is_list<T>::value;

/// A guarded sequence container is a reference counted sequence container, which guards the container against
/// deep content changes. Used together with views and locks, you can build logic where you can safely remove
/// content from the container, and continue iterating.
///
/// Use views to access the container elements. It is recommended to use guard locks when creating views. These
/// guards ensure that the container gets guarded before you try to access its content.
///
/// \tparam ContainerType The container to guard. This should be a sequence container such as std::vector<> or std::deque<>.
template <class ContainerType>
    requires guardable_sequence_container<ContainerType>
class GuardedSequenceContainer : public utils::ReferenceCountLockable<GuardedSequenceContainer<ContainerType>>
{
    using SelfType = GuardedSequenceContainer<ContainerType>;
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
    };

public:
    using GuardedContainer  = ContainerType;

    using value_type        = typename GuardedContainer::value_type;
    using reference         = typename GuardedContainer::reference;
    using const_reference   = typename GuardedContainer::const_reference;
    using size_type         = typename GuardedContainer::size_type;
    using difference_type   = typename GuardedContainer::difference_type;
    using pointer           = typename GuardedContainer::pointer;
    using const_pointer     = typename GuardedContainer::const_pointer;

    /// Creates a guarded vector container.
    explicit GuardedSequenceContainer(value_type invalidElement = value_type()) :
        m_invalidElement(std::move(invalidElement))
    {
    }

    /// \name Iterators
    /// \{

    /// The forward iterator of the guarded container.
    using iterator = IteratorWrap<SelfType, typename GuardedContainer::iterator, pointer, reference>;

    /// The forward const iterator of the guarded container.
    using const_iterator = IteratorWrap<const SelfType, typename GuardedContainer::const_iterator, const_pointer, const_reference>;

    /// The reverse iterator of the guarded container.
    using reverse_iterator = IteratorWrap<SelfType, typename GuardedContainer::reverse_iterator, pointer, reference>;

    /// The reverse const iterator of the guarded container.
    using const_reverse_iterator = IteratorWrap<const SelfType, typename GuardedContainer::const_reverse_iterator, const_pointer, const_reference>;

    iterator begin()
    {
        return iterator(m_container.begin(), m_container.end(), m_invalidElement);
    }
    const_iterator begin() const
    {
        return const_iterator(m_container.begin(), m_container.end(), m_invalidElement);
    }
    const_iterator cbegin() const
    {
        return const_iterator(m_container.cbegin(), m_container.cend(), m_invalidElement);
    }
    reverse_iterator rbegin()
    {
        return reverse_iterator(m_container.rbegin(), m_container.rend(), m_invalidElement);
    }
    const_reverse_iterator rbegin() const
    {
        return const_reverse_iterator(m_container.rbegin(), m_container.rend(), m_invalidElement);
    }
    const_reverse_iterator crbegin() const
    {
        return const_reverse_iterator(m_container.crbegin(), m_container.crend(), m_invalidElement);
    }
    iterator end()
    {
        return iterator(m_container.end(), m_container.end(), m_invalidElement);
    }
    const_iterator end() const
    {
        return const_iterator(m_container.end(), m_container.end(), m_invalidElement);
    }
    const_iterator cend() const
    {
        return const_iterator(m_container.cend(), m_container.cend(), m_invalidElement);
    }
    reverse_iterator rend()
    {
        return reverse_iterator(m_container.rend(), m_container.rend(), m_invalidElement);
    }
    const_reverse_iterator rend() const
    {
        return const_reverse_iterator(m_container.rend(), m_container.rend(), m_invalidElement);
    }
    const_reverse_iterator crend() const
    {
        return const_reverse_iterator(m_container.crend(), m_container.crend(), m_invalidElement);
    }

    /// Converts a const iterator to a non-const iterator.
    iterator toIterator(const_iterator position)
    {
        auto dist = std::distance(cbegin(), position);
        auto pos = begin();
        std::advance(pos, dist);
        return pos;
    }

    /// Converts a non-const iterator to a const iterator.
    const_iterator toConstIterator(iterator position)
    {
        auto dist = std::distance(begin(), position);
        auto pos = cbegin();
        std::advance(pos, dist);
        return pos;
    }

    /// The guarded view type of the container.
    using GuardedViewType = View<ContainerType, const_iterator>;
    using GuardedView = std::optional<GuardedViewType>;

    /// Returns the guarded view of the container.
    GuardedView getGuardedView() const
    {
        return m_guard;
    }

    /// \}

    /// Returns the size of the container, i.e. the number of valid elements in the container.
    size_type size() const
    {
        return std::distance(begin(), end());
    }

    /// \name Modifiers
    /// \{

    /// Clears the container. If the container is guarded, it resets the elements of the container,
    /// otgherwise it removes the elements of the container. The container will get cleared once it
    /// gets unguarded.
    void clear()
        // requires std::is_const_v<value_type>
    {
        if (m_guard)
        {
            std::for_each(m_container.begin(), m_container.end(),
                          [this](auto& item) { item = m_invalidElement; });
        }
        else
        {
            m_container.clear();
        }
    }

    /// Inserts a copy of an item at position. The operation fails if the insert position is inside
    /// the guarded view of the container.
    ///
    /// \param position The position where to insert the item.
    /// \param item The item to insert.
    /// \return On success, returns the iterator pointing to the inserted item. On failure returns a
    ///         nullopt.
    std::optional<iterator> insert(const_iterator position, const value_type& item)
    {
        if (m_guard && m_guard->inView(position))
        {
            return {};
        }

        auto result = m_container.insert(position, item);
        return iterator(result, m_container.end(), m_invalidElement);
    }

    /// Inserts an item at position. The operation fails if the insert position is inside
    /// the guarded view of the container.
    ///
    /// \param position The position where to insert the item.
    /// \param item The item to insert.
    /// \return On success, returns the iterator pointing to the inserted item. On failure returns a
    ///         std::nullopt.
    std::optional<iterator> insert(const_iterator position, value_type&& item)
    {
        if (m_guard && m_guard->inView(position))
        {
            return {};
        }

        auto result = m_container.insert(position, std::forward<value_type>(item));
        return iterator(result, m_container.end(), m_invalidElement);
    }

    /// Erases or resets the item at position.
    /// - The element is reset if the position is within the guarded view of the container.
    /// - The element is removed if the position is outside of the guarded view.
    ///
    /// \param position The position of the element to erase.
    /// \return The iterator which follows the erased element. If the container is guarded, and the
    ///         position falls outside of the locked view, returns std::nullopt.
    std::optional<iterator> erase(iterator position)
    {
        // Convert to const_iterator, then erase.
        auto pos = toConstIterator(position);
        return erase(pos);
    }

    /// Erases or resets the item at position.
    /// - The element is reset if the position is within the guarded view of the container.
    /// - The element is removed if the position is outside of the guarded view.
    ///
    /// \param position The position of the element to erase.
    /// \return The iterator which follows the erased element. If the container is guarded, and the
    ///         position falls outside of the locked view, returns std::nullopt.
    std::optional<iterator> erase(const_iterator position)
    {
        if (m_guard)
        {
            if (m_guard->inView(position))
            {
                auto pos = toIterator(position);
                *pos = m_invalidElement;
                return iterator(pos, m_container.end(), m_invalidElement);
            }

            // Erase outside of the view.
            m_container.erase(static_cast<typename ContainerType::const_iterator>(position));
            return {};
        }

        auto it = m_container.erase(static_cast<typename ContainerType::const_iterator>(position));
        return iterator(it, m_container.end(), m_invalidElement);
    }

    std::optional<iterator> erase(iterator first, iterator last)
    {
        if (m_guard)
        {
            auto baseFirst = static_cast<typename ContainerType::iterator>(first);
            auto baseLast = static_cast<typename ContainerType::iterator>(last);

            auto viewBegin = static_cast<typename ContainerType::iterator>(toIterator(m_guard->begin()));
            auto viewEnd = static_cast<typename ContainerType::iterator>(toIterator(m_guard->end()));

            // auto itBegin = first;
            // auto itEnd = last;

            // std::advance(itBegin, m_guard)



            // const auto firstIn = m_guard->inView(first);
            // const auto lastIn = m_guard->inView(last);

            // if (firstIn && lastIn)
            // {
            //     for (auto it = first; it != last; ++it)
            //     {
            //         *it = m_invalidElement;
            //     }
            //     return last;
            // }

            // Erase outside of the view.
            m_container.erase(static_cast<typename ContainerType::iterator>(first), static_cast<typename ContainerType::iterator>(last));
            return {};
        }

        auto it = m_container.erase(static_cast<typename ContainerType::iterator>(first), static_cast<typename ContainerType::iterator>(last));
        return iterator(it, m_container.end(), m_invalidElement);
    }

    std::optional<iterator> erase(const_iterator first, const_iterator last)
    {
        return erase(toIterator(first), toIterator(last));
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
    GuardedView m_guard;
    value_type m_invalidElement = {};

    /// Method called by ReferenceCountLockable<> when the container gets locked the first time.
    GuardedViewType acquireResources()
    {
        if (!m_guard)
        {
            m_guard = std::move(std::make_optional(GuardedViewType(cbegin(), cend())));
        }
        return *m_guard;
    }

    /// Method called by ReferenceCountLockable<> when the container gets fully unlocked.
    void releaseResources()
    {
        auto predicate = [this](auto& item) { return item == m_invalidElement; };
        m_container.erase(std::remove_if(m_container.begin(), m_container.end(), predicate), m_container.end());

        m_guard.reset();
    }
};

} // namespace utils

#endif // CONTAINERS_GUARDED_SEQUENCE_CONTAINER_HPP
