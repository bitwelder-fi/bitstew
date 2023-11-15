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

#ifndef UTILS_UTILITY_HPP
#define UTILS_UTILITY_HPP

#include <utility>

namespace utils
{

/// Template function to call a function \a f on an argument pack.
template <class Function, class... Arguments>
void for_each_arg(Function f, Arguments&&... args)
{
    (void)(int[]){(f(std::forward<Arguments>(args)), 0)...};
}

}

#endif
