/*
 * Copyright (C) 2024 bitWelder
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

#ifndef STEW_TYPE_TRAITS_HPP
#define STEW_TYPE_TRAITS_HPP

#include <deque>
#include <iterator>
#include <list>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace stew
{

/// \name Shared pointer tester
/// \{
template <typename T>
struct is_shared_pointer : std::false_type {};

template <typename T>
struct is_shared_pointer<std::shared_ptr<T>> : std::true_type {};

template <typename T>
inline constexpr bool is_shared_pointer_v = is_shared_pointer<T>::value;
/// \}

/// \name Weak pointer tester
/// \{
template <typename T>
struct is_weak_pointer : std::false_type {};

template <typename T>
struct is_weak_pointer<std::weak_ptr<T>> : std::true_type {};

template <typename T>
inline constexpr bool is_weak_pointer_v = is_weak_pointer<T>::value;
/// \}

/// \name Unique pointer tester
/// \{
template <typename T>
struct is_unique_pointer : std::false_type {};

template <typename T>
struct is_unique_pointer<std::unique_ptr<T>> : std::true_type {};

template <typename T>
inline constexpr bool is_unique_pointer_v = is_unique_pointer<T>::value;
/// \}

template <typename T>
inline constexpr bool is_smart_pointer_v = is_shared_pointer_v<T> || is_weak_pointer_v<T> || is_unique_pointer_v<T>;

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


template <typename T>
struct is_vector : std::false_type {};

template <typename T>
struct is_vector<std::vector<T>> : std::true_type {};


template <typename T>
struct is_deque : std::false_type {};

template <typename T>
struct is_deque<std::deque<T>> : std::true_type {};


template <typename T>
struct is_list : std::false_type {};

template <typename T>
struct is_list<std::list<T>> : std::true_type {};

/// \}

/// \name Standard library string and string_view tests.
/// \{
template <typename T>
struct is_std_string : std::false_type {};

template <>
struct is_std_string<std::string> : std::true_type {};

template <typename T>
inline constexpr bool is_std_string_v = is_std_string<T>::value;

template <typename T>
struct is_std_string_view : std::false_type {};

template <>
struct is_std_string_view<std::string_view> : std::true_type {};

template <typename T>
inline constexpr bool is_std_string_view_v = is_std_string_view<T>::value;
/// \}


/// \name Iterator testers
/// \{

template <class Iter, typename = void>
struct is_iterator : std::false_type {};

template <class Iter>
struct is_iterator<Iter, std::void_t<
                                    typename Iter::value_type,
                                    typename Iter::difference_type,
                                    typename Iter::iterator_category,
                                    decltype(++std::declval<Iter&>()), // incrementable
                                    decltype(*std::declval<Iter&>()), // dereferencable
                                    decltype(std::declval<Iter&>() == std::declval<Iter&>()) // comparable
                                     >> : std::true_type {};


template <class IT>
struct is_const_iterator
{
    static constexpr bool value = is_iterator<IT>::value && std::is_const_v<typename std::remove_pointer_t<typename std::iterator_traits<IT>::pointer>>;
};

template <typename IT>
struct is_reverse_iterator : std::false_type {};

template <typename IT>
struct is_reverse_iterator<std::reverse_iterator<IT>> : std::integral_constant<bool, !is_reverse_iterator<IT>::value> {};

/// \}

}

#endif // STEW_TYPE_TRAITS
