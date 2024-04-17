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

#include <stew/object_extensions/invokable.hpp>
#include <stew/object.hpp>

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
    STEW_LOG_INFO(__FUNCTION__);
}

int intNoArgs()
{
    STEW_LOG_INFO(__FUNCTION__);
    return 42;
}

void voidStringInt(std::string a1, int a2)
{
    STEW_LOG_INFO(__FUNCTION__ << ": " << a1 << ", " << a2);
}

void voidInvokableStringInt(stew::ObjectExtension* self, std::string a1, int a2)
{
    STEW_LOG_INFO(self->getName() << "(): " << a1 << ", " << a2);
}

int intStringInt(std::string a1, int a2)
{
    STEW_LOG_INFO(__FUNCTION__ << ": " << a1 << ", " << a2);
    return 42;
}


struct InvokableWrapper
{
    template<typename Function>
    explicit InvokableWrapper(std::string_view name, Function function) :
        m_invokable(stew::Invokable<Function, function>::create(name))
    {
    }

    stew::ObjectExtensionPtr operator->()
    {
        return m_invokable;
    }
    const stew::ObjectExtensionPtr operator->() const
    {
        return m_invokable;
    }
private:
    stew::ObjectExtensionPtr m_invokable;
};

struct Class
{
    void voidNoArgs()
    {
        STEW_LOG_INFO(__FUNCTION__);
    }

    int intNoArgs()
    {
        STEW_LOG_INFO(__FUNCTION__);
        return 42;
    }

    void voidStringInt(std::string a1, int a2)
    {
        STEW_LOG_INFO(__FUNCTION__ << ": " << a1 << ", " << a2);
    }

    int intStringInt(std::string a1, int a2)
    {
        STEW_LOG_INFO(__FUNCTION__ << ": " << a1 << ", " << a2);
        return 42;
    }

    void ptr(int* i)
    {
        ++(*i);
    }

    InvokableWrapper lambda{"lambda", [](stew::ObjectExtension* self) { STEW_LOG_INFO("invokable wrapper: " << self->getName()); }};
};

class TestObject : public stew::Object
{
public:
    explicit TestObject() :
        stew::Object("test")
    {
    }

    void voidWithInvokable(stew::ObjectExtension* self)
    {
        STEW_LOG_INFO(__FUNCTION__ << ": " << self->getName());
    }
};

}

TEST_F(InvokableTests, lambdaWithNoArgs)
{
    auto lambda = []() { STEW_LOG_INFO("lambdaWithNoArgs"); };
    using Callable = stew::Invokable<decltype(lambda), lambda>;
    auto callable = Callable::create("lambda");
    EXPECT_CALL(*m_mockPrinter, log("lambdaWithNoArgs"));
    auto result = callable->run();
    EXPECT_TRUE(result);
}

TEST_F(InvokableTests, lambdaWithSelf)
{
    auto lambda = [](stew::ObjectExtension* self)
    {
        STEW_LOG_INFO("lambdaWithSelf " << self->getName());
    };
    using Callable = stew::Invokable<decltype(lambda), lambda>;
    auto callable = Callable::create("lambda");
    EXPECT_CALL(*m_mockPrinter, log("lambdaWithSelf lambda"));
    auto result = callable->run();
    EXPECT_TRUE(result);
}

TEST_F(InvokableTests, callableWithNoArguments)
{
    using Callable = stew::Invokable<decltype(&voidNoArgs), voidNoArgs>;
    auto callable = Callable::create("voidNoArgs");
    EXPECT_CALL(*m_mockPrinter, log("voidNoArgs"));
    auto result = callable->run();
    EXPECT_TRUE(result);
}

TEST_F(InvokableTests, callableIntWithNoArguments)
{
    using Callable = stew::Invokable<decltype(&intNoArgs), intNoArgs>;
    auto callable = Callable::create("intNoArgs");
    EXPECT_CALL(*m_mockPrinter, log("intNoArgs"));
    auto result = callable->run();
    EXPECT_EQ(42, static_cast<int>(*result));
}

TEST_F(InvokableTests, callableWithArguments)
{
    using Callable = stew::Invokable<decltype(&voidStringInt), voidStringInt>;
    auto callable = Callable::create("voidStringInt");
    EXPECT_CALL(*m_mockPrinter, log("voidStringInt: one, 2"));
    auto arguments = stew::PackagedArguments(std::string("one"), 2);
    auto result = callable->run(arguments);
    EXPECT_TRUE(result);
}

TEST_F(InvokableTests, callableWithSelfAndArguments)
{
    using Callable = stew::Invokable<decltype(&voidInvokableStringInt), voidInvokableStringInt>;
    auto callable = Callable::create("voidInvokableStringInt");
    EXPECT_CALL(*m_mockPrinter, log("voidInvokableStringInt(): one, 2"));
    auto arguments = stew::PackagedArguments(std::string("one"), 2);
    auto result = callable->run(arguments);
    EXPECT_TRUE(result);
}

TEST_F(InvokableTests, callableIntWithArguments)
{
    using Callable = stew::Invokable<decltype(&intStringInt), intStringInt>;
    auto callable = Callable::create("intStringInt");
    EXPECT_CALL(*m_mockPrinter, log("intStringInt: one, 2"));
    auto arguments = stew::PackagedArguments(std::string("one"), 2);
    auto result = callable->run(arguments);
    EXPECT_EQ(42, static_cast<int>(*result));
}

TEST_F(InvokableTests, methodWithNoArguments)
{
    Class object;
    using Callable = stew::Invokable<decltype(&Class::voidNoArgs), &Class::voidNoArgs>;
    auto callable = Callable::create ("voidNoArgs");
    EXPECT_CALL(*m_mockPrinter, log("voidNoArgs"));
    auto arguments = stew::PackagedArguments(&object);
    auto result = callable->run(arguments);
    EXPECT_TRUE(result);
}

TEST_F(InvokableTests, methodIntWithNoArguments)
{
    Class object;
    using Callable = stew::Invokable<decltype(&Class::intNoArgs), &Class::intNoArgs>;
    auto callable = Callable::create("intNoArgs");
    EXPECT_CALL(*m_mockPrinter, log("intNoArgs"));
    auto arguments = stew::PackagedArguments(&object);
    auto result = callable->run(arguments);
    EXPECT_EQ(42, static_cast<int>(*result));
}

TEST_F(InvokableTests, methodWithInvokableArgument)
{
    auto object = std::make_shared<TestObject>();
    using Callable = stew::Invokable<decltype(&TestObject::voidWithInvokable), &TestObject::voidWithInvokable>;
    auto callable = Callable::create("voidWithInvokable");
    object->addExtension(callable);
    EXPECT_CALL(*m_mockPrinter, log("voidWithInvokable: voidWithInvokable"));
    auto result = callable->run(stew::PackagedArguments(std::string("one"), 2));
    EXPECT_TRUE(result);
}

TEST_F(InvokableTests, methodWithArguments)
{
    Class object;
    using Callable = stew::Invokable<decltype(&Class::voidStringInt), &Class::voidStringInt>;
    auto callable = Callable::create("voidStringInt");
    EXPECT_CALL(*m_mockPrinter, log("voidStringInt: one, 2"));
    auto arguments = stew::PackagedArguments(&object, std::string("one"), 2);
    auto result = callable->run(arguments);
    EXPECT_TRUE(result);
}

TEST_F(InvokableTests, methodIntWithArguments)
{
    Class object;
    using Callable = stew::Invokable<decltype(&Class::intStringInt), &Class::intStringInt>;
    auto callable = Callable::create("intStringInt");
    EXPECT_CALL(*m_mockPrinter, log("intStringInt: one, 2"));
    auto arguments = stew::PackagedArguments(&object, std::string("one"), 2);
    auto result = callable->run(arguments);
    EXPECT_EQ(42, static_cast<int>(*result));
}

TEST_F(InvokableTests, methodWithPointerArgument)
{
    Class object;
    using Callable = stew::Invokable<decltype(&Class::ptr), &Class::ptr>;
    auto callable = Callable::create("Class.ptr");
    int i = 41;
    auto arguments = stew::PackagedArguments(&object, &i);
    callable->run(arguments);
    EXPECT_EQ(42, i);
}

TEST_F(InvokableTests, invokeStaticInvokable)
{
    Class object;
    EXPECT_CALL(*m_mockPrinter, log("invokable wrapper: lambda"));
    object.lambda->run();
}
