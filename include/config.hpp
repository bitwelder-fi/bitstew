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

#ifndef CACHES_CONFIG_HPP
#define CACHES_CONFIG_HPP

#include <platform.hpp>

#define MAYBE_UNUSED(x)         (void)(x)
//
// disable copy construction and operator
//
#define DISABLE_COPY(Class) \
    Class(const Class&) = delete;\
    Class& operator=(const Class&) = delete;
#define DISABLE_MOVE(Class) \
    Class(Class&&) = delete; \
    Class& operator=(Class&&) = delete;

#define DISABLE_COPY_AND_MOVE(Class) \
    DISABLE_COPY(Class) \
    DISABLE_MOVE(Class)

#ifdef CONFIG_LIBRARY
#   define PROJECT_API     CACHES_DECL_EXPORT
#else
#   define PROJECT_API     CACHES_DECL_IMPORT
#endif
#define TEMPLATE_API


#ifdef DEBUG

#include <cassert>
#define abortIfFail(condition)  assert(condition)

#else

#include <cstdlib>
#define abortIfFail(condition)  if (!(condition)) { std::abort(); }

#endif

#endif // CACHES_CONFIG_HPP
