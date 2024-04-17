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

#ifndef ASSERT_HPP
#define ASSERT_HPP

#include <iostream>

#ifdef DEBUG

#include <cassert>

#define abortIfFail(condition)  assert(condition)
#define abortIfFailWithMessage(condition, message)  \
if (!(condition)) \
{ \
    std::cerr << message << std::endl; \
    assert(condition); \
}

#else

#include <cstdlib>
#define abortIfFail(condition)  if (!(condition)) { std::abort(); }
#define abortIfFailWithMessage(condition, message)  \
if (!(condition)) \
{ \
    std::cerr << message << std::endl; \
    std::abort(); \
}

#endif

#endif // ASSERT_HPP
