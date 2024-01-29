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

#include <meta/meta.hpp>
#include <meta/metadata/factory.hpp>
#include <meta/metadata/metaclass.hpp>
#include <meta/object.hpp>
#include <meta/library_config.hpp>

#include <meta/metadata/invokable.hpp>

#include "utils/domain_test_environment.hpp"

namespace
{

class AbstractClass : public meta::Object
{
public:
    META_CLASS("AbstractClass", AbstractClass, meta::Object)
    {
    };

    virtual void func() = 0;

protected:
    explicit AbstractClass(std::string_view name) :
        meta::Object(name)
    {
    }
};

class Interface
{
public:
    virtual ~Interface() = default;
    virtual void text() = 0;

    META_CLASS("Interface", Interface)
    {
    };
};

class OverrideClass : public meta::Object, public Interface
{
public:
    META_CLASS("AbstractClass", OverrideClass, meta::Object, Interface)
    {
    };

    virtual void func() = 0;

protected:
    explicit OverrideClass(std::string_view name) :
        meta::Object(name)
    {
    }
};

class PreObject : public AbstractClass
{
public:
    META_CLASS("PreObject", PreObject, AbstractClass)
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

    META_CLASS("Object", Object, PreObject, Interface)
    {
    };

    static std::shared_ptr<Object> create(std::string_view name, const meta::PackagedArguments&)
    {
        return std::shared_ptr<Object>(new Object(name));
    }

protected:
    explicit Object(std::string_view name) :
        PreObject(name)
    {
    }
};

class ObjectFactoryTest : public DomainTestEnvironment
{
protected:
    meta::ObjectFactory* m_factory = nullptr;
    std::size_t m_registrySize = 0u;

    void SetUp() override
    {
        initializeDomain(false, true);
        m_factory = meta::Library::instance().objectFactory();
        m_registrySize = std::distance(m_factory->begin(), m_factory->end());
    }
};

using MetaNameParam = std::tuple<std::string, bool>;
class MetaNameValidityTest : public ::testing::Test, public ::testing::WithParamInterface<MetaNameParam>
{
protected:
    std::string metaClassName;
    bool isValid;

    void SetUp() override
    {
        auto args = GetParam();
        metaClassName = std::get<std::string>(args);
        isValid = std::get<bool>(args);
    }
};

class MetaLibraryTest : public DomainTestEnvironment
{
protected:
    void SetUp() override
    {
        DomainTestEnvironment::initializeDomain(true, true);
    }
};

}

TEST(MetaClassTests, testMetaObject)
{
    ASSERT_NE(nullptr, meta::Object::getStaticMetaClass());
    EXPECT_FALSE(meta::Object::getStaticMetaClass()->isAbstract());
    EXPECT_EQ(0u, meta::Object::getStaticMetaClass()->getBaseClassCount());
}

TEST(MetaClassTests, testAbstractMetaClass)
{
    ASSERT_NE(nullptr, AbstractClass::getStaticMetaClass());
    EXPECT_TRUE(AbstractClass::getStaticMetaClass()->isAbstract());
    ASSERT_EQ(1u, AbstractClass::getStaticMetaClass()->getBaseClassCount());
    EXPECT_TRUE(AbstractClass::getStaticMetaClass()->isDerivedFromClass<meta::Object>());
    EXPECT_FALSE(AbstractClass::getStaticMetaClass()->isDerivedFromClass<AbstractClass>());
    EXPECT_TRUE(AbstractClass::getStaticMetaClass()->isDerivedFrom(*meta::Object::getStaticMetaClass()));
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
    EXPECT_TRUE(Object::getStaticMetaClass()->isDerivedFromClass<meta::Object>());
    EXPECT_TRUE(Object::getStaticMetaClass()->isDerivedFromClass<AbstractClass>());
    EXPECT_TRUE(Object::getStaticMetaClass()->isDerivedFromClass<Interface>());

    EXPECT_TRUE(Object::getStaticMetaClass()->isDerivedFrom(*meta::Object::getStaticMetaClass()));
    EXPECT_TRUE(Object::getStaticMetaClass()->isDerivedFrom(*AbstractClass::getStaticMetaClass()));
    EXPECT_TRUE(Object::getStaticMetaClass()->isDerivedFrom(*Interface::getStaticMetaClass()));
}

INSTANTIATE_TEST_SUITE_P(NameValidity, MetaNameValidityTest,
                         ::testing::Values(
                            MetaNameParam("meta.Object", true),
                            MetaNameParam("meta:Object", true),
                            MetaNameParam("meta-Object", true),
                            MetaNameParam("meta_Object", true),
                            MetaNameParam("meta~Object", false),
                            MetaNameParam("meta`Object", false),
                            MetaNameParam("meta!Object", false),
                            MetaNameParam("meta@Object", false),
                            MetaNameParam("meta#Object", false),
                            MetaNameParam("meta$Object", false),
                            MetaNameParam("meta$Object", false),
                            MetaNameParam("meta%Object", false),
                            MetaNameParam("meta^Object", false),
                            MetaNameParam("meta&Object", false),
                            MetaNameParam("meta*Object", false),
                            MetaNameParam("meta(Object", false),
                            MetaNameParam("meta)Object", false),
                            MetaNameParam("meta+Object", false),
                            MetaNameParam("meta=Object", false),
                            MetaNameParam("meta{Object", false),
                            MetaNameParam("meta[Object", false),
                            MetaNameParam("meta}Object", false),
                            MetaNameParam("meta]Object", false),
                            MetaNameParam("meta|Object", false),
                            MetaNameParam("meta\\Object", false),
                            MetaNameParam("meta;Object", false),
                            MetaNameParam("meta\"Object", false),
                            MetaNameParam("meta'Object", false),
                            MetaNameParam("meta<Object", false),
                            MetaNameParam("meta,Object", false),
                            MetaNameParam("meta>Object", false),
                            MetaNameParam("meta?Object", false),
                            MetaNameParam("meta/Object", false),
                            MetaNameParam("meta Object", false)));
TEST_P(MetaNameValidityTest, testMetaClassName)
{
    EXPECT_EQ(isValid, meta::isValidMetaName(this->metaClassName));
}

TEST_F(ObjectFactoryTest, testRegister)
{
    EXPECT_TRUE(m_factory->registerMetaClass(Object::getStaticMetaClass()));
    EXPECT_FALSE(m_factory->registerMetaClass(Object::getStaticMetaClass()));
}

TEST_F(ObjectFactoryTest, deepRegister)
{
    m_factory->registerMetaClass(Object::getStaticMetaClass());
    EXPECT_EQ(m_registrySize + 4u, std::distance(m_factory->begin(), m_factory->end()));
    EXPECT_NE(nullptr, m_factory->findMetaClass("Object"));
    EXPECT_NE(nullptr, m_factory->findMetaClass("PreObject"));
    EXPECT_NE(nullptr, m_factory->findMetaClass("Interface"));
    EXPECT_NE(nullptr, m_factory->findMetaClass("AbstractClass"));
    EXPECT_NE(nullptr, m_factory->findMetaClass("meta.Object"));
}

TEST_F(ObjectFactoryTest, testOverride)
{
    EXPECT_TRUE(m_factory->registerMetaClass(AbstractClass::getStaticMetaClass()));
    EXPECT_TRUE(m_factory->overrideMetaClass(OverrideClass::getStaticMetaClass()));
}

TEST_F(ObjectFactoryTest, deepOverride)
{
    EXPECT_TRUE(m_factory->registerMetaClass(AbstractClass::getStaticMetaClass()));
    EXPECT_EQ(m_registrySize + 1u, std::distance(m_factory->begin(), m_factory->end()));
    EXPECT_NE(nullptr, m_factory->findMetaClass("AbstractClass"));
    EXPECT_NE(nullptr, m_factory->findMetaClass("meta.Object"));

    EXPECT_TRUE(m_factory->overrideMetaClass(OverrideClass::getStaticMetaClass()));
    EXPECT_EQ(m_registrySize + 2u, std::distance(m_factory->begin(), m_factory->end()));
    EXPECT_NE(nullptr, m_factory->findMetaClass("AbstractClass"));
    EXPECT_NE(nullptr, m_factory->findMetaClass("meta.Object"));
    EXPECT_NE(nullptr, m_factory->findMetaClass("Interface"));
}

TEST_F(ObjectFactoryTest, testFindMetaClass)
{
    EXPECT_TRUE(m_factory->registerMetaClass(AbstractClass::getStaticMetaClass()));
    EXPECT_TRUE(m_factory->registerMetaClass(Interface::getStaticMetaClass()));
    EXPECT_TRUE(m_factory->registerMetaClass(Object::getStaticMetaClass()));

    EXPECT_EQ(Interface::getStaticMetaClass(), m_factory->findMetaClass("Interface"));
}

TEST_F(ObjectFactoryTest, testMetaClassCreate)
{
    m_factory->registerMetaClass(Object::getStaticMetaClass());

    auto metaClass = m_factory->findMetaClass("Object");
    ASSERT_EQ(Object::getStaticMetaClass(), metaClass);
    ASSERT_NE(nullptr, metaClass->create("doing"));
    auto castedObject = metaClass->create<Object>("next");
    ASSERT_NE(nullptr, castedObject);
}

TEST_F(ObjectFactoryTest, testMetaClassCastedCreate)
{
    m_factory->registerMetaClass(Object::getStaticMetaClass());
    auto metaClass = m_factory->findMetaClass("Object");
    auto castedObject = metaClass->create<Object>("next");
    EXPECT_NE(nullptr, castedObject);
}

TEST_F(MetaLibraryTest, testDomainHasObjectFactory)
{
    EXPECT_NE(nullptr, meta::Library::instance().objectFactory());
}

TEST_F(MetaLibraryTest, testDomainObjectFactoryRegistryContent)
{
    ASSERT_NE(nullptr, meta::Library::instance().objectFactory());
    EXPECT_NE(nullptr, meta::Library::instance().objectFactory()->findMetaClass("meta.Object"));
}
