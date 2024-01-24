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
#include <meta/object.hpp>

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

using InvokableTests = CallableTestBase;

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

void voidInvokableStringInt(meta::Invokable* self, std::string a1, int a2)
{
    META_LOG_INFO(self->getName() << "(): " << a1 << ", " << a2);
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

class TestObject : public meta::Object
{
public:
    explicit TestObject() :
        meta::Object("test")
    {
    }

    void voidWithInvokable(meta::Invokable* self)
    {
        META_LOG_INFO(__FUNCTION__ << ": " << self->getName());
    }
};

}

TEST_F(InvokableTests, lambdaWithNoArgs)
{
    auto lambda = []() { META_LOG_INFO("lambdaWithNoArgs"); };
    auto callable = meta::Invokable::create("lambda", lambda);
    EXPECT_CALL(*m_mockPrinter, log("lambdaWithNoArgs"));
    auto result = callable->execute();
    EXPECT_FALSE(result.has_value());
}

TEST_F(InvokableTests, lambdaWithSelf)
{
    auto lambda = [](meta::Invokable* self)
    {
        META_LOG_INFO("lambdaWithSelf " << self->getName());
    };
    auto callable = meta::Invokable::create("lambda", lambda);
    EXPECT_CALL(*m_mockPrinter, log("lambdaWithSelf lambda"));
    auto result = callable->execute();
    EXPECT_FALSE(result.has_value());
}

TEST_F(InvokableTests, callableWithNoArguments)
{
    auto callable = meta::Invokable::create("voidNoArgs", voidNoArgs);
    EXPECT_CALL(*m_mockPrinter, log("voidNoArgs"));
    auto result = callable->execute();
    EXPECT_FALSE(result.has_value());
}

TEST_F(InvokableTests, callableIntWithNoArguments)
{
    auto callable = meta::Invokable::create("intNoArgs", intNoArgs);
    EXPECT_CALL(*m_mockPrinter, log("intNoArgs"));
    auto result = callable->execute();
    EXPECT_EQ(42, static_cast<int>(result));
}

TEST_F(InvokableTests, callableWithArguments)
{
    auto callable = meta::Invokable::create("voidStringInt", voidStringInt);
    EXPECT_CALL(*m_mockPrinter, log("voidStringInt: one, 2"));
    auto arguments = meta::PackagedArguments(std::string("one"), 2);
    auto result = callable->execute(arguments);
    EXPECT_FALSE(result.has_value());
}

TEST_F(InvokableTests, callableWithSelfAndArguments)
{
    auto callable = meta::Invokable::create("voidInvokableStringInt", voidInvokableStringInt);
    EXPECT_CALL(*m_mockPrinter, log("voidInvokableStringInt(): one, 2"));
    auto arguments = meta::PackagedArguments(std::string("one"), 2);
    auto result = callable->execute(arguments);
    EXPECT_FALSE(result.has_value());
}

TEST_F(InvokableTests, callableIntWithArguments)
{
    auto callable = meta::Invokable::create("intStringInt", intStringInt);
    EXPECT_CALL(*m_mockPrinter, log("intStringInt: one, 2"));
    auto arguments = meta::PackagedArguments(std::string("one"), 2);
    auto result = callable->execute(arguments);
    EXPECT_EQ(42, static_cast<int>(result));
}

TEST_F(InvokableTests, methodWithNoArguments)
{
    Class object;
    auto callable = meta::Invokable::create ("voidNoArgs", &Class::voidNoArgs);
    EXPECT_CALL(*m_mockPrinter, log("voidNoArgs"));
    auto arguments = meta::PackagedArguments(&object);
    auto result = callable->execute(arguments);
    EXPECT_FALSE(result.has_value());
}

TEST_F(InvokableTests, methodIntWithNoArguments)
{
    Class object;
    auto callable = meta::Invokable::create("intNoArgs", &Class::intNoArgs);
    EXPECT_CALL(*m_mockPrinter, log("intNoArgs"));
    auto arguments = meta::PackagedArguments(&object);
    auto result = callable->execute(arguments);
    EXPECT_EQ(42, static_cast<int>(result));
}

TEST_F(InvokableTests, methodWithInvokableArgument)
{
    auto object = std::make_shared<TestObject>();
    auto callable = meta::Invokable::create("voidWithInvokable", &TestObject::voidWithInvokable);
    object->addExtension(callable);
    EXPECT_CALL(*m_mockPrinter, log("voidWithInvokable: voidWithInvokable"));
    auto result = callable->execute(meta::PackagedArguments(std::string("one"), 2));
    EXPECT_FALSE(result.has_value());
}

TEST_F(InvokableTests, methodWithArguments)
{
    Class object;
    auto callable = meta::Invokable::create("voidStringInt", &Class::voidStringInt);
    EXPECT_CALL(*m_mockPrinter, log("voidStringInt: one, 2"));
    auto arguments = meta::PackagedArguments(&object, std::string("one"), 2);
    auto result = callable->execute(arguments);
    EXPECT_FALSE(result.has_value());
}

TEST_F(InvokableTests, methodIntWithArguments)
{
    Class object;
    auto callable = meta::Invokable::create("intStringInt", &Class::intStringInt);
    EXPECT_CALL(*m_mockPrinter, log("intStringInt: one, 2"));
    auto arguments = meta::PackagedArguments(&object, std::string("one"), 2);
    auto result = callable->execute(arguments);
    EXPECT_EQ(42, static_cast<int>(result));
}

TEST_F(InvokableTests, methodWithPointerArgument)
{
    Class object;
    auto callable = meta::Invokable::create("Class.ptr", &Class::ptr);
    int i = 41;
    auto arguments = meta::PackagedArguments(&object, &i);
    callable->execute(arguments);
    EXPECT_EQ(42, i);
}
