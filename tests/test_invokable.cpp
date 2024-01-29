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
        m_invokable(meta::InvokableType<Function>::create(name, function))
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
    using Invokable = meta::InvokableType<decltype(lambda)>;
    auto callable = Invokable::create("lambda", lambda);
    EXPECT_CALL(*m_mockPrinter, log("lambdaWithNoArgs"));
    auto result = callable->execute();
    EXPECT_FALSE(result.has_value());
}

TEST_F(InvokableTests, lambdaWithSelf)
{
    auto lambda = [](meta::ObjectExtension* self)
    {
        META_LOG_INFO("lambdaWithSelf " << self->getName());
    };
    using Invokable = meta::InvokableType<decltype(lambda)>;
    auto callable = Invokable::create("lambda", lambda);
    EXPECT_CALL(*m_mockPrinter, log("lambdaWithSelf lambda"));
    auto result = callable->execute();
    EXPECT_FALSE(result.has_value());
}

TEST_F(InvokableTests, callableWithNoArguments)
{
    using Invokable = meta::InvokableType<decltype(&voidNoArgs)>;
    auto callable = Invokable::create("voidNoArgs", voidNoArgs);
    EXPECT_CALL(*m_mockPrinter, log("voidNoArgs"));
    auto result = callable->execute();
    EXPECT_FALSE(result.has_value());
}

TEST_F(InvokableTests, callableIntWithNoArguments)
{
    using Invokable = meta::InvokableType<decltype(&intNoArgs)>;
    auto callable = Invokable::create("intNoArgs", intNoArgs);
    EXPECT_CALL(*m_mockPrinter, log("intNoArgs"));
    auto result = callable->execute();
    EXPECT_EQ(42, static_cast<int>(result));
}

TEST_F(InvokableTests, callableWithArguments)
{
    using Invokable = meta::InvokableType<decltype(&voidStringInt)>;
    auto callable = Invokable::create("voidStringInt", voidStringInt);
    EXPECT_CALL(*m_mockPrinter, log("voidStringInt: one, 2"));
    auto arguments = meta::PackagedArguments(std::string("one"), 2);
    auto result = callable->execute(arguments);
    EXPECT_FALSE(result.has_value());
}

TEST_F(InvokableTests, callableWithSelfAndArguments)
{
    using Invokable = meta::InvokableType<decltype(&voidInvokableStringInt)>;
    auto callable = Invokable::create("voidInvokableStringInt", voidInvokableStringInt);
    EXPECT_CALL(*m_mockPrinter, log("voidInvokableStringInt(): one, 2"));
    auto arguments = meta::PackagedArguments(std::string("one"), 2);
    auto result = callable->execute(arguments);
    EXPECT_FALSE(result.has_value());
}

TEST_F(InvokableTests, callableIntWithArguments)
{
    using Invokable = meta::InvokableType<decltype(&intStringInt)>;
    auto callable = Invokable::create("intStringInt", intStringInt);
    EXPECT_CALL(*m_mockPrinter, log("intStringInt: one, 2"));
    auto arguments = meta::PackagedArguments(std::string("one"), 2);
    auto result = callable->execute(arguments);
    EXPECT_EQ(42, static_cast<int>(result));
}

TEST_F(InvokableTests, methodWithNoArguments)
{
    Class object;
    using Invokable = meta::InvokableType<decltype(&Class::voidNoArgs)>;
    auto callable = Invokable::create ("voidNoArgs", &Class::voidNoArgs);
    EXPECT_CALL(*m_mockPrinter, log("voidNoArgs"));
    auto arguments = meta::PackagedArguments(&object);
    auto result = callable->execute(arguments);
    EXPECT_FALSE(result.has_value());
}

TEST_F(InvokableTests, methodIntWithNoArguments)
{
    Class object;
    using Invokable = meta::InvokableType<decltype(&Class::intNoArgs)>;
    auto callable = Invokable::create("intNoArgs", &Class::intNoArgs);
    EXPECT_CALL(*m_mockPrinter, log("intNoArgs"));
    auto arguments = meta::PackagedArguments(&object);
    auto result = callable->execute(arguments);
    EXPECT_EQ(42, static_cast<int>(result));
}

TEST_F(InvokableTests, methodWithInvokableArgument)
{
    auto object = std::make_shared<TestObject>();
    using Invokable = meta::InvokableType<decltype(&TestObject::voidWithInvokable)>;
    auto callable = Invokable::create("voidWithInvokable", &TestObject::voidWithInvokable);
    object->addExtension(callable);
    EXPECT_CALL(*m_mockPrinter, log("voidWithInvokable: voidWithInvokable"));
    auto result = callable->execute(meta::PackagedArguments(std::string("one"), 2));
    EXPECT_FALSE(result.has_value());
}

TEST_F(InvokableTests, methodWithArguments)
{
    Class object;
    using Invokable = meta::InvokableType<decltype(&Class::voidStringInt)>;
    auto callable = Invokable::create("voidStringInt", &Class::voidStringInt);
    EXPECT_CALL(*m_mockPrinter, log("voidStringInt: one, 2"));
    auto arguments = meta::PackagedArguments(&object, std::string("one"), 2);
    auto result = callable->execute(arguments);
    EXPECT_FALSE(result.has_value());
}

TEST_F(InvokableTests, methodIntWithArguments)
{
    Class object;
    using Invokable = meta::InvokableType<decltype(&Class::intStringInt)>;
    auto callable = Invokable::create("intStringInt", &Class::intStringInt);
    EXPECT_CALL(*m_mockPrinter, log("intStringInt: one, 2"));
    auto arguments = meta::PackagedArguments(&object, std::string("one"), 2);
    auto result = callable->execute(arguments);
    EXPECT_EQ(42, static_cast<int>(result));
}

TEST_F(InvokableTests, methodWithPointerArgument)
{
    Class object;
    using Invokable = meta::InvokableType<decltype(&Class::ptr)>;
    auto callable = Invokable::create("Class.ptr", &Class::ptr);
    int i = 41;
    auto arguments = meta::PackagedArguments(&object, &i);
    callable->execute(arguments);
    EXPECT_EQ(42, i);
}

TEST_F(InvokableTests, invokeStaticInvokable)
{
    Class object;
    EXPECT_CALL(*m_mockPrinter, log("invokable wrapper: lambda"));
    object.lambda->execute();
}


struct MetaExtension
{
    template <class Function>
    explicit MetaExtension(std::string name, Function function)
    {
        factory = [name, function]()
        {
            return meta::InvokableType<Function>::create(name, function);
        };
    }

    meta::ObjectExtensionPtr create()
    {
        return factory();
    }
    template <class ObjectExtensionType>
    std::shared_ptr<ObjectExtensionType> create()
    {
        return std::dynamic_pointer_cast<ObjectExtensionType>(factory());
    }

private:
    std::function<meta::ObjectExtensionPtr()> factory;
};

TEST_F(InvokableTests, invokableType)
{
    MetaExtension metaInvokableFactory("voidStringInt", &Class::voidStringInt);
    auto metaInvokable = metaInvokableFactory.create();

    Class object;
    EXPECT_CALL(*m_mockPrinter, log("voidStringInt: one, 2"));
    auto arguments = meta::PackagedArguments(&object, std::string("one"), 2);
    auto result = metaInvokable->execute(arguments);
    EXPECT_FALSE(result.has_value());
}
