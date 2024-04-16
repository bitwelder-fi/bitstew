/*
 * Copyright (C) 2017-2019 bitWelder
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

#ifndef META_ALGORITHM_HPP
#define META_ALGORITHM_HPP

namespace meta
{

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-parameter"

/// Template function to call a function \a f on an argument pack.
template <class Function, class... Arguments>
void for_each_arg(Function f, Arguments... args)
{
    (f(args),...);
}
#pragma GCC diagnostic pop

/// Vector utility, loops a \a predicate through a \a vector.
template <typename Type, typename Allocator, typename Predicate>
void for_each(std::vector<Type, Allocator>& v, const Predicate& predicate)
{
    std::for_each(v.begin(), v.end(), predicate);
}
template <typename Type, typename Allocator, typename Predicate>
void for_each(const std::vector<Type, Allocator>& v, const Predicate& predicate)
{
    std::for_each(v.begin(), v.end(), predicate);
}

/// Vector utility, finds a \a value in a \a vector
template <typename Type, typename Allocator, typename VType>
decltype(auto) find(std::vector<Type, Allocator>& v, const VType& value)
{
    return std::find(v.begin(), v.end(), value);
}

/// Vector utility, loops a \a predicate through a \a vector.
template <typename Type, typename Allocator, typename Predicate>
decltype(auto) find_if(std::vector<Type, Allocator>& v, const Predicate& predicate)
{
    return find_if(v.begin(), v.end(), predicate);
}
template <typename Type, typename Allocator, typename Predicate>
decltype(auto) find_if(const std::vector<Type, Allocator>& v, const Predicate& predicate)
{
    return find_if(v.begin(), v.end(), predicate);
}

/// Vector utility, removes the occurences of \a value from a \a vector.
template <typename Type, typename Allocator, typename VType>
void erase(std::vector<Type, Allocator>& v, const VType& value)
{
    v.erase(remove(v.begin(), v.end(), value), v.end());
}

/// Vector utility, removes the first occurence of \a value from a \a vector.
template <typename Type, typename Allocator, typename VType>
void erase_first(std::vector<Type, Allocator>& v, const VType& value)
{
    auto it = find(v.begin(), v.end(), value);
    if (it != v.end())
    {
        v.erase(it);
    }
}

/// Vector utility, removes the occurences for which the predicate gives affirmative result.
template <typename Type, typename Allocator, typename Predicate>
void erase_if(std::vector<Type, Allocator>& v, const Predicate& predicate)
{
    v.erase(remove_if(v.begin(), v.end(), predicate), v.end());
}

}

#endif
