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

#ifndef CONTAINERS_VIEW_HPP
#define CONTAINERS_VIEW_HPP

#include <meta/meta_api.hpp>
#include <type_traits>

namespace containers
{

struct META_API forward_t {};
struct META_API forward_const_t {};
struct META_API reverse_t {};
struct META_API reverse_const_t {};


/// Gets a view of a container and retains the container. The view may differ from the locked view of
/// the container.
template <typename GuardedContainerType, typename IteratorType = forward_t>
struct View
{
    using Iterator  = std::conditional_t<std::is_same_v<IteratorType, forward_t>,
                                        typename GuardedContainerType::Iterator,
                                        std::conditional_t<std::is_same_v<IteratorType, forward_const_t>,
                                                           typename GuardedContainerType::ConstIterator,
                                                           std::conditional_t<std::is_same_v<IteratorType, reverse_t>,
                                                                              typename GuardedContainerType::ReverseIterator,
                                                                              typename GuardedContainerType::ConstReverseIterator>>>;
    using ViewType  = typename GuardedContainerType::template View<Iterator>;

    /// Constructor, locks the guarded container and returns an iterator range to the locked content.
    explicit View(GuardedContainerType& container) :
        m_container(container),
        m_containerView(m_container)
    {

        m_container.retain();
    }
    /// Destructor
    ~View()
    {
        m_container.release();
    }

    /// The begin iterator of the locked container view.
    Iterator begin() const
    {
        return m_containerView.begin();
    }
    /// The end iterator of the locked container view.
    Iterator end() const
    {
        return m_containerView.end();
    }

private:
    GuardedContainerType& m_container;
    ViewType m_containerView;
};


/// Locks a container, and gets the locked view of it. The view of the container is always the locked
/// view, no matter how many times the container gets retained with using this view.
template <typename GuardedContainerType>
struct LockView
{
    using Iterator  = typename GuardedContainerType::Iterator;
    using ViewType  = typename GuardedContainerType::template View<Iterator>;

    /// Constructor, locks the guarded container and returns an iterator range to the locked content.
    explicit LockView(GuardedContainerType& container) :
        m_container(container),
        m_lockedView(m_container.retain())
    {
    }
    /// Destructor
    ~LockView()
    {
        m_container.release();
    }

    /// The begin iterator of the locked container view.
    Iterator begin() const
    {
        return m_lockedView.begin();
    }
    /// The end iterator of the locked container view.
    Iterator end() const
    {
        return m_lockedView.end();
    }

    /// Cast to view type of the container.
    operator ViewType()
    {
        return m_lockedView;
    }

private:
    GuardedContainerType& m_container;
    ViewType m_lockedView;
};

}

#endif // UTILS_CONTAINER_VIEW_HPP
