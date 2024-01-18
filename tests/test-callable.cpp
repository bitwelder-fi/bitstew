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

#include <meta/metadata/invokable.hpp>

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

using Invokable = CallableTestBase;

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

    void ptr(int* i)
    {
        ++(*i);
    }
};

}


TEST_F(Invokable, validCallable)
{
    meta::Invokable callable;
    EXPECT_FALSE(callable.isValid());
}

TEST_F(Invokable, callableWithNoArguments)
{
    meta::Invokable callable("voidNoArgs", voidNoArgs);
    EXPECT_CALL(*m_mockPrinter, log("voidNoArgs"));
    auto arguments = meta::PackagedArguments();
    auto result = callable.apply(arguments);
    EXPECT_FALSE(result.has_value());
}

TEST_F(Invokable, callableIntWithNoArguments)
{
    meta::Invokable callable("intNoArgs", intNoArgs);
    EXPECT_CALL(*m_mockPrinter, log("intNoArgs"));
    auto arguments = meta::PackagedArguments();
    auto result = callable.apply(arguments);
    EXPECT_EQ(42, static_cast<int>(result));
}

TEST_F(Invokable, callableWithArguments)
{
    meta::Invokable callable("voidStringInt", voidStringInt);
    EXPECT_CALL(*m_mockPrinter, log("voidStringInt: one, 2"));
    auto arguments = meta::PackagedArguments(std::string("one"), 2);
    auto result = callable.apply(arguments);
    EXPECT_FALSE(result.has_value());
}

TEST_F(Invokable, callableIntWithArguments)
{
    meta::Invokable callable("intStringInt", intStringInt);
    EXPECT_CALL(*m_mockPrinter, log("intStringInt: one, 2"));
    auto arguments = meta::PackagedArguments(std::string("one"), 2);
    auto result = callable.apply(arguments);
    EXPECT_EQ(42, static_cast<int>(result));
}

TEST_F(Invokable, methodWithNoArguments)
{
    Class object;
    meta::Invokable callable("voidNoArgs", &Class::voidNoArgs);
    EXPECT_CALL(*m_mockPrinter, log("voidNoArgs"));
    auto arguments = meta::PackagedArguments(&object);
    auto result = callable.apply(arguments);
    EXPECT_FALSE(result.has_value());
}

TEST_F(Invokable, methodIntWithNoArguments)
{
    Class object;
    meta::Invokable callable("intNoArgs", &Class::intNoArgs);
    EXPECT_CALL(*m_mockPrinter, log("intNoArgs"));
    auto arguments = meta::PackagedArguments(&object);
    auto result = callable.apply(arguments);
    EXPECT_EQ(42, static_cast<int>(result));
}

TEST_F(Invokable, methodWithArguments)
{
    Class object;
    meta::Invokable callable("voidStringInt", &Class::voidStringInt);
    EXPECT_CALL(*m_mockPrinter, log("voidStringInt: one, 2"));
    auto arguments = meta::PackagedArguments(&object, std::string("one"), 2);
    auto result = callable.apply(arguments);
    EXPECT_FALSE(result.has_value());
}

TEST_F(Invokable, methodIntWithArguments)
{
    Class object;
    meta::Invokable callable("intStringInt", &Class::intStringInt);
    EXPECT_CALL(*m_mockPrinter, log("intStringInt: one, 2"));
    auto arguments = meta::PackagedArguments(&object, std::string("one"), 2);
    auto result = callable.apply(arguments);
    EXPECT_EQ(42, static_cast<int>(result));
}

TEST_F(Invokable, methodWithPointerArgument)
{
    Class object;
    meta::Invokable callable("Class.ptr", &Class::ptr);
    int i = 41;
    auto arguments = meta::PackagedArguments(&object, &i);
    callable.apply(arguments);
    EXPECT_EQ(42, i);
}

TEST_F(Invokable, moveCallable)
{
    meta::Invokable callable1("intNoArgs", intNoArgs);
    meta::Invokable callable2;
    EXPECT_TRUE(callable1.isValid());
    EXPECT_FALSE(callable2.isValid());

    callable2 = std::move(callable1);
    EXPECT_FALSE(callable1.isValid());
    EXPECT_TRUE(callable2.isValid());
}
