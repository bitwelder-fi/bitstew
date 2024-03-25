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

#include <algorithm>

namespace containers
{

/// Gets a view of a container and retains the container. The view may differ from the locked view of
/// the container.
template <class GuardedContainerType, class IteratorType>
struct View
{
    using iterator_type = IteratorType;
    using value_type    = typename GuardedContainerType::value_type;

    /// Constructor, locks the guarded container and returns an iterator range to the locked content.
    explicit View(iterator_type begin, iterator_type end) :
        m_viewBegin(begin),
        m_viewEnd(end)
    {
    }
    /// Destructor
    ~View()
    {
    }

    /// The begin iterator of the locked container view.
    iterator_type begin() const
    {
        return m_viewBegin;
    }
    /// The end iterator of the locked container view.
    iterator_type end() const
    {
        return m_viewEnd;
    }

    /// Find an item in the view.
    /// \param item The item to find in the view.
    /// \return On success, returns the position of the itemn within the view. On failure, returns
    ///         \e std::nullopt.
    iterator_type find(const value_type& item)
    {
        return std::find(m_viewBegin, m_viewEnd, item);
    }

    /// Returns the size of the view. The size is the number of valid elements in the view.
    std::size_t size() const
    {
        return std::distance(m_viewBegin, m_viewEnd);
    }

    template <typename TIteratorType>
    bool inView(TIteratorType position)
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

    iterator_type erase(iterator_type pos)
    {
        abortIfFail(inView(pos));


    }

private:
    iterator_type m_viewBegin;
    iterator_type m_viewEnd;
};


/// Locks a container, and gets the locked view of it. The view of the container is always the locked
/// view, no matter how many times the container gets retained with using this view.
template <class GuardedContainerType>
struct LockView
{
    using view_type     = typename GuardedContainerType::LockedViewType;
    using iterator_type = typename view_type::iterator_type;
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
    iterator_type begin() const
    {
        return m_lockedView.begin();
    }
    /// The end iterator of the locked container view.
    iterator_type end() const
    {
        return m_lockedView.end();
    }

    /// Find an item in the view.
    /// \param item The item to find in the view.
    /// \return On success, returns the position of the itemn within the view. On failure, returns
    ///         \e std::nullopt.
    iterator_type find(const value_type& item)
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
    view_type m_lockedView;
};

}

#endif // UTILS_CONTAINER_VIEW_HPP
