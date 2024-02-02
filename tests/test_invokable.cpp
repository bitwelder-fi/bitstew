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

void voidInvokableStringInt(meta::ObjectExtension* self, std::string a1, int a2)
{
    META_LOG_INFO(self->getName() << "(): " << a1 << ", " << a2);
}

int intStringInt(std::string a1, int a2)
{
    META_LOG_INFO(__FUNCTION__ << ": " << a1 << ", " << a2);
    return 42;
}


struct InvokableWrapper
{
    template<typename Function>
    explicit InvokableWrapper(std::string_view name, Function function) :
        m_invokable(meta::Invokable<Function, function>::create(name))
    {
    }

    meta::ObjectExtensionPtr operator->()
    {
        return m_invokable;
    }
    const meta::ObjectExtensionPtr operator->() const
    {
        return m_invokable;
    }
private:
    meta::ObjectExtensionPtr m_invokable;
};

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

    InvokableWrapper lambda{"lambda", [](meta::ObjectExtension* self) { META_LOG_INFO("invokable wrapper: " << self->getName()); }};
};

class TestObject : public meta::Object
{
public:
    explicit TestObject() :
        meta::Object("test")
    {
    }

    void voidWithInvokable(meta::ObjectExtension* self)
    {
        META_LOG_INFO(__FUNCTION__ << ": " << self->getName());
    }
};

}

TEST_F(InvokableTests, lambdaWithNoArgs)
{
    auto lambda = []() { META_LOG_INFO("lambdaWithNoArgs"); };
    using Callable = meta::Invokable<decltype(lambda), lambda>;
    auto callable = Callable::create("lambda");
    EXPECT_CALL(*m_mockPrinter, log("lambdaWithNoArgs"));
    auto result = callable->run();
    EXPECT_FALSE(result.has_value());
}

TEST_F(InvokableTests, lambdaWithSelf)
{
    auto lambda = [](meta::ObjectExtension* self)
    {
        META_LOG_INFO("lambdaWithSelf " << self->getName());
    };
    using Callable = meta::Invokable<decltype(lambda), lambda>;
    auto callable = Callable::create("lambda");
    EXPECT_CALL(*m_mockPrinter, log("lambdaWithSelf lambda"));
    auto result = callable->run();
    EXPECT_FALSE(result.has_value());
}

TEST_F(InvokableTests, callableWithNoArguments)
{
    using Callable = meta::Invokable<decltype(&voidNoArgs), voidNoArgs>;
    auto callable = Callable::create("voidNoArgs");
    EXPECT_CALL(*m_mockPrinter, log("voidNoArgs"));
    auto result = callable->run();
    EXPECT_FALSE(result.has_value());
}

TEST_F(InvokableTests, callableIntWithNoArguments)
{
    using Callable = meta::Invokable<decltype(&intNoArgs), intNoArgs>;
    auto callable = Callable::create("intNoArgs");
    EXPECT_CALL(*m_mockPrinter, log("intNoArgs"));
    auto result = callable->run();
    EXPECT_EQ(42, static_cast<int>(result));
}

TEST_F(InvokableTests, callableWithArguments)
{
    using Callable = meta::Invokable<decltype(&voidStringInt), voidStringInt>;
    auto callable = Callable::create("voidStringInt");
    EXPECT_CALL(*m_mockPrinter, log("voidStringInt: one, 2"));
    auto arguments = meta::PackagedArguments(std::string("one"), 2);
    auto result = callable->run(arguments);
    EXPECT_FALSE(result.has_value());
}

TEST_F(InvokableTests, callableWithSelfAndArguments)
{
    using Callable = meta::Invokable<decltype(&voidInvokableStringInt), voidInvokableStringInt>;
    auto callable = Callable::create("voidInvokableStringInt");
    EXPECT_CALL(*m_mockPrinter, log("voidInvokableStringInt(): one, 2"));
    auto arguments = meta::PackagedArguments(std::string("one"), 2);
    auto result = callable->run(arguments);
    EXPECT_FALSE(result.has_value());
}

TEST_F(InvokableTests, callableIntWithArguments)
{
    using Callable = meta::Invokable<decltype(&intStringInt), intStringInt>;
    auto callable = Callable::create("intStringInt");
    EXPECT_CALL(*m_mockPrinter, log("intStringInt: one, 2"));
    auto arguments = meta::PackagedArguments(std::string("one"), 2);
    auto result = callable->run(arguments);
    EXPECT_EQ(42, static_cast<int>(result));
}

TEST_F(InvokableTests, methodWithNoArguments)
{
    Class object;
    using Callable = meta::Invokable<decltype(&Class::voidNoArgs), &Class::voidNoArgs>;
    auto callable = Callable::create ("voidNoArgs");
    EXPECT_CALL(*m_mockPrinter, log("voidNoArgs"));
    auto arguments = meta::PackagedArguments(&object);
    auto result = callable->run(arguments);
    EXPECT_FALSE(result.has_value());
}

TEST_F(InvokableTests, methodIntWithNoArguments)
{
    Class object;
    using Callable = meta::Invokable<decltype(&Class::intNoArgs), &Class::intNoArgs>;
    auto callable = Callable::create("intNoArgs");
    EXPECT_CALL(*m_mockPrinter, log("intNoArgs"));
    auto arguments = meta::PackagedArguments(&object);
    auto result = callable->run(arguments);
    EXPECT_EQ(42, static_cast<int>(result));
}

TEST_F(InvokableTests, methodWithInvokableArgument)
{
    auto object = std::make_shared<TestObject>();
    using Callable = meta::Invokable<decltype(&TestObject::voidWithInvokable), &TestObject::voidWithInvokable>;
    auto callable = Callable::create("voidWithInvokable");
    object->addExtension(callable);
    EXPECT_CALL(*m_mockPrinter, log("voidWithInvokable: voidWithInvokable"));
    auto result = callable->run(meta::PackagedArguments(std::string("one"), 2));
    EXPECT_FALSE(result.has_value());
}

TEST_F(InvokableTests, methodWithArguments)
{
    Class object;
    using Callable = meta::Invokable<decltype(&Class::voidStringInt), &Class::voidStringInt>;
    auto callable = Callable::create("voidStringInt");
    EXPECT_CALL(*m_mockPrinter, log("voidStringInt: one, 2"));
    auto arguments = meta::PackagedArguments(&object, std::string("one"), 2);
    auto result = callable->run(arguments);
    EXPECT_FALSE(result.has_value());
}

TEST_F(InvokableTests, methodIntWithArguments)
{
    Class object;
    using Callable = meta::Invokable<decltype(&Class::intStringInt), &Class::intStringInt>;
    auto callable = Callable::create("intStringInt");
    EXPECT_CALL(*m_mockPrinter, log("intStringInt: one, 2"));
    auto arguments = meta::PackagedArguments(&object, std::string("one"), 2);
    auto result = callable->run(arguments);
    EXPECT_EQ(42, static_cast<int>(result));
}

TEST_F(InvokableTests, methodWithPointerArgument)
{
    Class object;
    using Callable = meta::Invokable<decltype(&Class::ptr), &Class::ptr>;
    auto callable = Callable::create("Class.ptr");
    int i = 41;
    auto arguments = meta::PackagedArguments(&object, &i);
    callable->run(arguments);
    EXPECT_EQ(42, i);
}

TEST_F(InvokableTests, invokeStaticInvokable)
{
    Class object;
    EXPECT_CALL(*m_mockPrinter, log("invokable wrapper: lambda"));
    object.lambda->run();
}
