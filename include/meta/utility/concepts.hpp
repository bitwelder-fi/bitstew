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

#ifndef UTIL_CONCEPTS_HPP
#define UTIL_CONCEPTS_HPP

#include <concepts>
#include <meta/utility/type_traits.hpp>

namespace concepts
{

template <class P>
concept raw_pointer = std::is_pointer_v<P>;

template <class S>
concept smart_pointer = traits::is_smart_pointer_v<S>;

template <class P>
concept unique_pointer = traits::is_unique_pointer_v<P>;

template <class T>
concept std_string = traits::is_std_string_v<T>;

template <class T>
concept std_string_view = traits::is_std_string_view_v<T>;

}

#endif
