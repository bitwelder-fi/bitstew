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

#ifndef UTIL_TYPE_TRAITS_HPP
#define UTIL_TYPE_TRAITS_HPP

#include <memory>
#include <type_traits>

namespace traits
{

/// \name Shared pointer tester
/// \{
template <typename T>
struct is_shared_ptr : std::false_type {};

template <typename T>
struct is_shared_ptr<std::shared_ptr<T>> : std::true_type {};
/// \}

/// \name Weak pointer tester
/// \{
template <typename T>
struct is_weak_ptr : std::false_type {};

template <typename T>
struct is_weak_ptr<std::weak_ptr<T>> : std::true_type {};
/// \}

/// \name Unique pointer tester
/// \{
template <typename T>
struct is_unique_ptr : std::false_type {};

template <typename T>
struct is_unique_ptr<std::unique_ptr<T>> : std::true_type {};
/// \}

template <typename T>
inline constexpr bool is_smart_pointer_v = is_shared_ptr<T>::value || is_weak_ptr<T>::value || is_unique_ptr<T>::value;

/// \name Container test.
/// \{
template <typename T, typename = void>
struct is_container : std::false_type {};

template <typename T>
struct is_container<T, std::void_t<
                           typename T::value_type,
                           typename T::size_type,
                           typename T::allocator_type,
                           typename T::iterator,
                           typename T::const_iterator,
                           decltype(std::declval<T>().size()),
                           decltype(std::declval<T>().begin()),
                           decltype(std::declval<T>().end())
                           >> : std::true_type {};

/// Helper variable template
template <typename T>
inline constexpr bool is_container_v = is_container<T>::value;

/// \}

}

#endif // UTIL_TYPE_TRAITS
