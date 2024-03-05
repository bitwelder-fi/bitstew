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

#ifndef UTILS_CONTAINER_VIEW_HPP
#define UTILS_CONTAINER_VIEW_HPP

namespace utils
{

/// Locks a container, and gets the locked view of it.
template <typename GuardedContainerType>
struct ContainerView
{
    using IteratorType = typename GuardedContainerType::Iterator;
    using ViewType     = typename GuardedContainerType::template View<IteratorType>;

    /// Constructor, locks the guarded container and returns an iterator range to the locked content.
    explicit ContainerView(GuardedContainerType& container) :
        m_container(container),
        m_lockedView(m_container.lock())
    {
    }
    /// Destructor
    ~ContainerView()
    {
        m_container.unlock();
    }

    /// The begin iterator of the locked container view.
    IteratorType begin() const
    {
        return m_lockedView.begin();
    }
    /// The end iterator of the locked container view.
    IteratorType end() const
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
