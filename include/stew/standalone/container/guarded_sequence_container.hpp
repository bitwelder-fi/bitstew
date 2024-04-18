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

#ifndef STEW_GUARDED_SEQUENCE_CONTAINER_HPP
#define STEW_GUARDED_SEQUENCE_CONTAINER_HPP

#include <stew/standalone/container/iterator.hpp>
#include <stew/standalone/container/view.hpp>
#include <stew/standalone/utility/concepts.hpp>
#include <stew/standalone/utility/type_traits.hpp>
#include <stew/standalone/utility/reference_counted.hpp>

#include <algorithm>
#include <cmath>
#include <optional>
#include <utility>

namespace containers
{

namespace
{

template <class T>
concept generic_resetable = !stew::smart_pointer<T>;

template <class T>
concept guardable_sequence_container = !stew::is_list<T>::value;

}

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

    /// \name Invalid element
    /// \{

    /// Initializes an element as invalid from the point of view of the container.
    /// \param element The smart pointer element to invalidate.
    void invalidate(stew::smart_pointer auto& element)
    {
        element.reset();
    }
    /// Initializes an element as invalid from the point of view of the container.
    /// \param element The non-smart pointer element to invalidate.
    void invalidate(generic_resetable auto& element)
    {
        element = m_invalidElement;
    }

    /// Checks whether an element is considered as invalid by the container.
    /// Floating-point version.
    /// \param element The element value to check.
    /// \return If the element is valid, returns \e true, otherwise \e false.
    bool isValid(const std::floating_point auto& element) const
    {
        return std::isnan(m_invalidElement) ? !std::isnan(element) : m_invalidElement != element;
    }

    /// Checks whether an element is considered as invalid by the container.
    /// \param element The element value to check.
    /// \return If the element is valid, returns \e true, otherwise \e false.
    bool isValid(const non_floating_point auto& element) const
    {
        return m_invalidElement != element;
    }
    /// \}

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
        return iterator(*this, m_container.begin(), m_container.end());
    }
    const_iterator begin() const
    {
        return cbegin();
    }
    const_iterator cbegin() const
    {
        return const_iterator(*this, m_container.cbegin(), m_container.cend());
    }
    reverse_iterator rbegin()
    {
        return reverse_iterator(*this, m_container.rbegin(), m_container.rend());
    }
    const_reverse_iterator rbegin() const
    {
        return crbegin();
    }
    const_reverse_iterator crbegin() const
    {
        return const_reverse_iterator(*this, m_container.crbegin(), m_container.crend());
    }
    iterator end()
    {
        return iterator(*this, m_container.end(), m_container.end());
    }
    const_iterator end() const
    {
        return cend();
    }
    const_iterator cend() const
    {
        return const_iterator(*this, m_container.cend(), m_container.cend());
    }
    reverse_iterator rend()
    {
        return reverse_iterator(*this, m_container.rend(), m_container.rend());
    }
    const_reverse_iterator rend() const
    {
        return crend();
    }
    const_reverse_iterator crend() const
    {
        return const_reverse_iterator(*this, m_container.crend(), m_container.crend());
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

    /// Returns the effective size of the container, which is the size of the guarded container that
    /// includes the invalid elements.
    size_type effectiveSize() const
    {
        return m_container.size();
    }

    /// \name Modifiers
    /// \{

    /// Clears the container. If the container is guarded, it resets the elements of the container,
    /// otgherwise it removes the elements of the container. The container will get cleared once it
    /// gets unguarded.
    void clear()
    {
        if (m_guard)
        {
            std::for_each(m_container.begin(), m_container.end(),
                          [this](auto& item) { resetElement(item); });
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
        requires std::is_copy_constructible_v<value_type>
    {
        if (m_guard && m_guard->inView(position))
        {
            return {};
        }

        auto result = m_container.insert(position, item);
        return iterator(*this, result, m_container.end());
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
        return iterator(*this, result, m_container.end());
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
        return erase(static_cast<const_iterator>(position));
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
                resetElement(*pos);
                return iterator(*this, pos, m_container.end());
            }

            // Erase outside of the view.
            m_container.erase(static_cast<typename ContainerType::const_iterator>(position));
            return {};
        }

        auto it = m_container.erase(static_cast<typename ContainerType::const_iterator>(position));
        return iterator(*this, it, m_container.end());
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

    void resetElement(stew::smart_pointer auto& element)
    {
        element.reset();
    }
    void resetElement(generic_resetable auto& element)
    {
        element = m_invalidElement;
    }

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
        auto predicate = [this](auto& item) { return !this->isValid(item); };
        m_container.erase(std::remove_if(m_container.begin(), m_container.end(), predicate), m_container.end());

        m_guard.reset();
    }
};

} // namespace containers

#endif // CONTAINERS_GUARDED_SEQUENCE_CONTAINER_HPP
