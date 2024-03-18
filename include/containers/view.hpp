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

namespace containers
{

/// Gets a view of a container and retains the container. The view may differ from the locked view of
/// the container.
template <typename GuardedContainerType, typename Iterator = typename GuardedContainerType::Iterator>
struct View
{
    using ViewType      = typename GuardedContainerType::template View<Iterator>;
    using value_type    = typename GuardedContainerType::value_type;

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

    /// Find an item in the view.
    /// \param item The item to find in the view.
    /// \return On success, returns the position of the itemn within the view. On failure, returns
    ///         \e std::nullopt.
    Iterator find(const value_type& item)
    {
        return m_containerView.find(item);
    }

    /// Returns the size of the view. The size is the number of valid elements in the view.
    std::size_t size() const
    {
        return m_containerView.size();
    }

private:
    GuardedContainerType& m_container;
    ViewType m_containerView;
};


/// Locks a container, and gets the locked view of it. The view of the container is always the locked
/// view, no matter how many times the container gets retained with using this view.
template <typename GuardedContainerType, typename Iterator = typename GuardedContainerType::Iterator>
struct LockView
{
    using ViewType      = typename GuardedContainerType::template View<Iterator>;
    using value_type    = typename GuardedContainerType::value_type;

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

    /// Find an item in the view.
    /// \param item The item to find in the view.
    /// \return On success, returns the position of the itemn within the view. On failure, returns
    ///         \e std::nullopt.
    Iterator find(const value_type& item)
    {
        return m_lockedView.find(item);
    }

    /// Returns the size of the view. The size is the number of valid elements in the view.
    std::size_t size() const
    {
        return m_lockedView.size();
    }

private:
    GuardedContainerType& m_container;
    ViewType m_lockedView;
};

}

#endif // UTILS_CONTAINER_VIEW_HPP
