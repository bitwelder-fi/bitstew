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
#include <concepts>
#include <ranges>

namespace containers
{

namespace
{

template <class IL, class IR>
concept same_iterator = std::same_as<IL, IR>;

// template <class IL, class IR>
// concept lconst_iterator = std::is_const_v<IL> &&
//                           !std::is_const_v<IR> &&
//                           std::same_as<std::remove_cvref_t<IL>, std::remove_cv_t<IR>>;

// template <class IL, class IR>
// concept rconst_iterator = !std::is_const_v<IL> &&
//                           std::is_const_v<IR> &&
//                           std::same_as<std::remove_cvref_t<IL>, std::remove_cv_t<IR>>;

}

/// Gets a view of a container and retains the container. The view may differ from the locked view of
/// the container.
template <class GuardedContainerType, class IteratorType>
struct View : public std::ranges::view_interface<View<GuardedContainerType, IteratorType>>
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

    /// Checks whether an iterator is within the view.
    /// \tparam TIteratorType The iterator type of the position. This variant requires that the iterator
    ///         template argument and this iterator are of the same type.
    /// \param position The position to check.
    /// \return If the position is within the view, returns \e true, otherwise \e false.
    template <typename TIteratorType = IteratorType>
        requires same_iterator<TIteratorType, IteratorType>
    bool inView(TIteratorType position)
    {
        return position >= m_viewBegin && position < m_viewEnd;
    }

    // /// Checks whether an iterator is within the view.
    // /// \tparam TIteratorType The iterator type of the position. This variant requires that the iterator
    // ///         template argument and this iterator are of the same type, but this is const iterator and
    // ///         the template argument is not.
    // /// \param position The position to check.
    // /// \return If the position is within the view, returns \e true, otherwise \e false.
    // template <typename TIteratorType = IteratorType>
    //     requires lconst_iterator<IteratorType, TIteratorType>
    // bool inView(TIteratorType position)
    // {
    //     const auto cposition = static_cast<TIteratorType>(position);
    //     return cposition >= m_viewBegin && cposition < m_viewEnd;
    // }

    // /// Checks whether an iterator is within the view.
    // /// \tparam TIteratorType The iterator type of the position. This variant requires that the iterator
    // ///         template argument and this iterator are of the same type, but this is not a const iterator
    // ///         and the template argument is.
    // /// \param position The position to check.
    // /// \return If the position is within the view, returns \e true, otherwise \e false.
    // template <typename TIteratorType = IteratorType>
    //     requires rconst_iterator<IteratorType, TIteratorType>
    // bool inView(TIteratorType position)
    // {
    //     return position >= static_cast<TIteratorType>(m_viewBegin) && position < static_cast<TIteratorType>(m_viewEnd);
    // }

private:
    iterator_type m_viewBegin;
    iterator_type m_viewEnd;
};


/// Locks a container, and gets the locked view of it. The view of the container is always the locked
/// view, no matter how many times the container gets retained with using this view.
template <class GuardedContainerType>
struct LockView
{
    using view_type     = typename GuardedContainerType::GuardedViewType;
    using iterator_type = typename view_type::iterator_type;
    using value_type    = typename GuardedContainerType::value_type;

    /// Constructor, locks the guarded container and returns an iterator range to the locked content.
    explicit LockView(GuardedContainerType& container) :
        m_container(container),
        m_guard(m_container.retain())
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
        return m_guard.begin();
    }
    /// The end iterator of the locked container view.
    iterator_type end() const
    {
        return m_guard.end();
    }

    /// Find an item in the view.
    /// \param item The item to find in the view.
    /// \return On success, returns the position of the itemn within the view. On failure, returns
    ///         \e std::nullopt.
    iterator_type find(const value_type& item)
    {
        return m_guard.find(item);
    }

    /// Returns the content size of the view. The size is the number of valid elements in the view.
    std::size_t size() const
    {
        return m_guard.size();
    }

private:
    GuardedContainerType& m_container;
    view_type m_guard;
};

}

#endif // UTILS_CONTAINER_VIEW_HPP
