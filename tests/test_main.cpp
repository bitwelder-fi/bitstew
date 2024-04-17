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

#include <gtest/gtest.h>
#include <gtest/gtest-spi.h>
#include <gmock/gmock.h>

#include <stew/stew.hpp>
#include <stew/library_config.hpp>

#include <thread>

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    GTEST_FLAG_SET(death_test_style, "fast");

    int result = 0;
    std::thread testRunner([&result] { result = RUN_ALL_TESTS(); });
    testRunner.join();

    delete ::testing::internal::GetFailureReporter();

    return result;
}
