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

#include "utils/domain_test_environment.hpp"

#include <meta/metadata/callable.hpp>

namespace
{

class CallableTestBase : public DomainTestEnvironment
{
protected:
    void SetUp() override
    {
        // Single threaded
        initializeDomain(false, true);
    }
};

using Callable = CallableTestBase;

void voidNoArgs()
{
    META_LOG_INFO(__FUNCTION__);
}

int intNoArgs()
{
    META_LOG_INFO(__FUNCTION__);
    return 42;
}

void voidStringInt(std::string a1, int a2)
{
    META_LOG_INFO(__FUNCTION__ << ": " << a1 << ", " << a2);
}

int intStringInt(std::string a1, int a2)
{
    META_LOG_INFO(__FUNCTION__ << ": " << a1 << ", " << a2);
    return 42;
}

struct Class
{
    void voidNoArgs()
    {
        META_LOG_INFO(__FUNCTION__);
    }

    int intNoArgs()
    {
        META_LOG_INFO(__FUNCTION__);
        return 42;
    }

    void voidStringInt(std::string a1, int a2)
    {
        META_LOG_INFO(__FUNCTION__ << ": " << a1 << ", " << a2);
    }

    int intStringInt(std::string a1, int a2)
    {
        META_LOG_INFO(__FUNCTION__ << ": " << a1 << ", " << a2);
        return 42;
    }

    void ref(int* i)
    {
        ++(*i);
    }
};

}


TEST_F(Callable, validCallable)
{
    meta::Callable callable;
    EXPECT_FALSE(callable.isValid());
}

TEST_F(Callable, callableWithNoArguments)
{
    meta::Callable callable("voidNoArgs", voidNoArgs);
    EXPECT_CALL(*m_mockPrinter, log("voidNoArgs"));
    auto arguments = meta::PackagedArguments();
    auto result = callable.apply(arguments);
    EXPECT_TRUE(result == std::nullopt);
}

TEST_F(Callable, callableIntWithNoArguments)
{
    meta::Callable callable("intNoArgs", intNoArgs);
    EXPECT_CALL(*m_mockPrinter, log("intNoArgs"));
    auto arguments = meta::PackagedArguments();
    auto result = callable.apply(arguments);
    EXPECT_EQ(42, *result);
}

TEST_F(Callable, callableWithArguments)
{
    meta::Callable callable("voidStringInt", voidStringInt);
    EXPECT_CALL(*m_mockPrinter, log("voidStringInt: one, 2"));
    auto arguments = meta::PackagedArguments(std::string("one"), 2);
    auto result = callable.apply(arguments);
    EXPECT_TRUE(result == std::nullopt);
}

TEST_F(Callable, callableIntWithArguments)
{
    meta::Callable callable("intStringInt", intStringInt);
    EXPECT_CALL(*m_mockPrinter, log("intStringInt: one, 2"));
    auto arguments = meta::PackagedArguments(std::string("one"), 2);
    auto result = callable.apply(arguments);
    EXPECT_EQ(42, *result);
}

TEST_F(Callable, methodWithNoArguments)
{
    Class object;
    meta::Callable callable("voidNoArgs", &Class::voidNoArgs);
    EXPECT_CALL(*m_mockPrinter, log("voidNoArgs"));
    auto arguments = meta::PackagedArguments(&object);
    auto result = callable.apply(arguments);
    EXPECT_TRUE(result == std::nullopt);
}

TEST_F(Callable, methodIntWithNoArguments)
{
    Class object;
    meta::Callable callable("intNoArgs", &Class::intNoArgs);
    EXPECT_CALL(*m_mockPrinter, log("intNoArgs"));
    auto arguments = meta::PackagedArguments(&object);
    auto result = callable.apply(arguments);
    EXPECT_EQ(42, *result);
}

TEST_F(Callable, methodWithArguments)
{
    Class object;
    meta::Callable callable("voidStringInt", &Class::voidStringInt);
    EXPECT_CALL(*m_mockPrinter, log("voidStringInt: one, 2"));
    auto arguments = meta::PackagedArguments(&object, std::string("one"), 2);
    auto result = callable.apply(arguments);
    EXPECT_TRUE(result == std::nullopt);
}

TEST_F(Callable, methodIntWithArguments)
{
    Class object;
    meta::Callable callable("intStringInt", &Class::intStringInt);
    EXPECT_CALL(*m_mockPrinter, log("intStringInt: one, 2"));
    auto arguments = meta::PackagedArguments(&object, std::string("one"), 2);
    auto result = callable.apply(arguments);
    EXPECT_EQ(42, *result);
}

TEST_F(Callable, methodWithRefArgument)
{
    Class object;
    meta::Callable callable("Class.ref", &Class::ref);
    int i = 41;
    auto arguments = meta::PackagedArguments(&object, &i);
    callable.apply(arguments);
    EXPECT_EQ(42, i);
}

TEST_F(Callable, moveCallable)
{
    meta::Callable callable1("intNoArgs", intNoArgs);
    meta::Callable callable2;
    EXPECT_TRUE(callable1.isValid());
    EXPECT_FALSE(callable2.isValid());

    callable2 = std::move(callable1);
    EXPECT_FALSE(callable1.isValid());
    EXPECT_TRUE(callable2.isValid());
}
