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

#include <gtest/gtest.h>
#include "utils/trace_printer_mock.hpp"

#include <meta/meta.hpp>
#include <meta/metadata/factory.hpp>
#include <meta/metadata/metaclass.hpp>
#include <meta/metadata/metaobject.hpp>
#include <meta/library_config.hpp>

#include <meta/metadata/callable.hpp>

namespace
{

class AbstractClass : public meta::MetaObject
{
public:
    AbstractMetaData(AbstractClass, meta::MetaObject)
    {
    };

    virtual void func() = 0;

protected:
    explicit AbstractClass(std::string_view name) :
        meta::MetaObject(name)
    {
    }
};

class Interface
{
public:
    virtual ~Interface() = default;
    virtual void text() = 0;

    AbstractBaseMetaData(Interface)
    {
    };
};

class PreObject : public AbstractClass
{
public:
    AbstractMetaData(PreObject, AbstractClass)
    {
    };
    virtual void func3() = 0;

protected:
    explicit PreObject(std::string_view name) :
        AbstractClass(name)
    {
    }
};

class Object : public PreObject, public Interface
{
public:
    void func() override
    {}
    void text() override
    {}
    void func3() final
    {}

    static inline meta::Callable metaFunc{"Object.func", &Object::func};

    MetaData(Object, PreObject, Interface)
    {
        // meta::Callable("Object.func", &Object::func);
    };

    static std::shared_ptr<Object> create(std::string_view name)
    {
        return std::shared_ptr<Object>(new Object(name));
    }

protected:
    explicit Object(std::string_view name) :
        PreObject(name)
    {
    }
};

class ObjectFactoryTest : public ::testing::Test
{
protected:
    std::unique_ptr<meta::ObjectFactory> m_factory;
    void SetUp() override
    {
        m_factory = std::make_unique<meta::ObjectFactory>();
    }

    void TearDown() override
    {
        m_factory.reset();
    }
};

using MetaClassNameParam = std::tuple<std::string, bool>;
class MetaClassNameValidityTest : public ObjectFactoryTest, public ::testing::WithParamInterface<MetaClassNameParam>
{
protected:
    std::string metaClassName;
    bool isValid;

    void SetUp() override
    {
        auto config = meta::LibraryArguments();
        config.taskScheduler.createThreadPool = false;
        meta::Domain::instance().initialize(config);

        // mock the logging
        auto logger = std::make_shared<MockPrinter>();
        meta::Domain::instance().tracer()->clearTracePrinters();
        meta::Domain::instance().tracer()->addTracePrinter(logger);

        ObjectFactoryTest::SetUp();

        auto args = GetParam();
        metaClassName = get<std::string>(args);
        isValid = get<bool>(args);

        if (!isValid)
        {
            auto argument = "Invalid meta class name: " + metaClassName;
            EXPECT_CALL(*logger, log(argument));
        }
    }
};

class MetaDomainTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        meta::Domain::instance().initialize(meta::LibraryArguments());
    }
    void TearDown() override
    {
        meta::Domain::instance().uninitialize();
    }
};

}

TEST(MetaClassTests, testMetaObject)
{
    ASSERT_NE(nullptr, meta::MetaObject::getStaticMetaClass());
    EXPECT_FALSE(meta::MetaObject::getStaticMetaClass()->isAbstract());
    EXPECT_EQ(0u, meta::MetaObject::getStaticMetaClass()->getBaseClassCount());
}

TEST(MetaClassTests, testAbstractMetaClass)
{
    ASSERT_NE(nullptr, AbstractClass::getStaticMetaClass());
    EXPECT_TRUE(AbstractClass::getStaticMetaClass()->isAbstract());
    ASSERT_EQ(1u, AbstractClass::getStaticMetaClass()->getBaseClassCount());
    EXPECT_TRUE(AbstractClass::getStaticMetaClass()->isDerivedFromClass<meta::MetaObject>());
    EXPECT_FALSE(AbstractClass::getStaticMetaClass()->isDerivedFromClass<AbstractClass>());
    EXPECT_TRUE(AbstractClass::getStaticMetaClass()->isDerivedFrom(*meta::MetaObject::getStaticMetaClass()));
    EXPECT_TRUE(AbstractClass::getStaticMetaClass()->isDerivedFrom(*AbstractClass::getStaticMetaClass()));
}

TEST(MetaClassTests, testInterface)
{
    ASSERT_NE(nullptr, Interface::getStaticMetaClass());
    EXPECT_TRUE(Interface::getStaticMetaClass()->isAbstract());
    ASSERT_EQ(0u, Interface::getStaticMetaClass()->getBaseClassCount());
}

TEST(MetaClassTests, testObject)
{
    ASSERT_NE(nullptr, Object::getStaticMetaClass());
    EXPECT_FALSE(Object::getStaticMetaClass()->isAbstract());
    ASSERT_EQ(2u, Object::getStaticMetaClass()->getBaseClassCount());
    EXPECT_TRUE(Object::getStaticMetaClass()->isDerivedFromClass<meta::MetaObject>());
    EXPECT_TRUE(Object::getStaticMetaClass()->isDerivedFromClass<AbstractClass>());
    EXPECT_TRUE(Object::getStaticMetaClass()->isDerivedFromClass<Interface>());

    EXPECT_TRUE(Object::getStaticMetaClass()->isDerivedFrom(*meta::MetaObject::getStaticMetaClass()));
    EXPECT_TRUE(Object::getStaticMetaClass()->isDerivedFrom(*AbstractClass::getStaticMetaClass()));
    EXPECT_TRUE(Object::getStaticMetaClass()->isDerivedFrom(*Interface::getStaticMetaClass()));
}

INSTANTIATE_TEST_SUITE_P(NameValidity, MetaClassNameValidityTest,
                         ::testing::Values(
                            MetaClassNameParam("meta.Object", true),
                            MetaClassNameParam("meta:Object", true),
                            MetaClassNameParam("meta-Object", true),
                            MetaClassNameParam("meta_Object", true),
                            MetaClassNameParam("meta~Object", false),
                            MetaClassNameParam("meta`Object", false),
                            MetaClassNameParam("meta!Object", false),
                            MetaClassNameParam("meta@Object", false),
                            MetaClassNameParam("meta#Object", false),
                            MetaClassNameParam("meta$Object", false),
                            MetaClassNameParam("meta$Object", false),
                            MetaClassNameParam("meta%Object", false),
                            MetaClassNameParam("meta^Object", false),
                            MetaClassNameParam("meta&Object", false),
                            MetaClassNameParam("meta*Object", false),
                            MetaClassNameParam("meta(Object", false),
                            MetaClassNameParam("meta)Object", false),
                            MetaClassNameParam("meta+Object", false),
                            MetaClassNameParam("meta=Object", false),
                            MetaClassNameParam("meta{Object", false),
                            MetaClassNameParam("meta[Object", false),
                            MetaClassNameParam("meta}Object", false),
                            MetaClassNameParam("meta]Object", false),
                            MetaClassNameParam("meta|Object", false),
                            MetaClassNameParam("meta\\Object", false),
                            MetaClassNameParam("meta;Object", false),
                            MetaClassNameParam("meta\"Object", false),
                            MetaClassNameParam("meta'Object", false),
                            MetaClassNameParam("meta<Object", false),
                            MetaClassNameParam("meta,Object", false),
                            MetaClassNameParam("meta>Object", false),
                            MetaClassNameParam("meta?Object", false),
                            MetaClassNameParam("meta/Object", false),
                            MetaClassNameParam("meta Object", false)));
TEST_P(MetaClassNameValidityTest, testMetaClassName)
{
    EXPECT_EQ(isValid, m_factory->registerMetaClass(metaClassName, Interface::getStaticMetaClass()));
}

TEST_F(ObjectFactoryTest, testRegister)
{
    EXPECT_TRUE(m_factory->registerMetaClass("meta.Object", Object::getStaticMetaClass()));
    EXPECT_FALSE(m_factory->registerMetaClass("meta.Object", Object::getStaticMetaClass()));
}

TEST_F(ObjectFactoryTest, testOverride)
{
    EXPECT_TRUE(m_factory->registerMetaClass("meta.Object", AbstractClass::getStaticMetaClass()));
    EXPECT_TRUE(m_factory->overrideMetaClass("meta.Object", Object::getStaticMetaClass()));
}

TEST_F(ObjectFactoryTest, testFindMetaClass)
{
    EXPECT_TRUE(m_factory->registerMetaClass("meta.AbstractClass", AbstractClass::getStaticMetaClass()));
    EXPECT_TRUE(m_factory->registerMetaClass("meta.Interface", Interface::getStaticMetaClass()));
    EXPECT_TRUE(m_factory->registerMetaClass("meta.Object", Object::getStaticMetaClass()));

    EXPECT_EQ(Interface::getStaticMetaClass(), m_factory->findMetaClass("meta.Interface"));
}

TEST_F(ObjectFactoryTest, testMetaClassCreate)
{
    m_factory->registerMetaClass("meta.Object", Object::getStaticMetaClass());

    auto metaClass = m_factory->findMetaClass("meta.Object");
    ASSERT_EQ(Object::getStaticMetaClass(), metaClass);
    ASSERT_NE(nullptr, metaClass->create("doing"));
    auto castedObject = metaClass->create<Object>("next");
    ASSERT_NE(nullptr, castedObject);
}

TEST_F(ObjectFactoryTest, testMetaClassCastedCreate)
{
    m_factory->registerMetaClass("meta.Object", Object::getStaticMetaClass());
    auto metaClass = m_factory->findMetaClass("meta.Object");
    auto castedObject = metaClass->create<Object>("next");
    EXPECT_NE(nullptr, castedObject);
}

TEST_F(MetaDomainTest, testDomainHasObjectFactory)
{
    EXPECT_NE(nullptr, meta::Domain::instance().objectFactory());
}

TEST_F(MetaDomainTest, testDomainObjectFactoryRegistryContent)
{
    ASSERT_NE(nullptr, meta::Domain::instance().objectFactory());
    EXPECT_NE(nullptr, meta::Domain::instance().objectFactory()->findMetaClass("meta.MetaObject"));
}
