/*
 * Copyright (C) 2023 bitWelder
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

#ifndef TEST_UTILS_HPP
#define TEST_UTILS_HPP

#include <stew/core/preprocessor.hpp>
#include <gtest/gtest.h>
#include <gtest/gtest-spi.h>

struct SafeDeathTestScope
{
    explicit SafeDeathTestScope() :
        previousDeathTestStyle(::testing::GTEST_FLAG(death_test_style))
#ifndef PREPROCESSOR_CPP_MSVC
        , previousDebughFlagpreviousDebughFlag(_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF))
#endif
    {
        GTEST_FLAG_SET(death_test_style, "threadsafe");
    }
    ~SafeDeathTestScope()
    {
#ifndef PREPROCESSOR_CPP_MSVC
        _CrtSetDbgFlag(previousDebughFlags);
#endif
        GTEST_FLAG_SET(death_test_style, previousDeathTestStyle);
    }

private:
    std::string previousDeathTestStyle;
#ifndef PREPROCESSOR_CPP_MSVC
    int previousDebughFlags = 0;
#endif
};

#endif // TEST_UTILS_HPP
