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

#include "test_meta_class_fixtures.hpp"

namespace
{

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

TEST(MetaData, size)
{
    std::cerr << "MetaExtensionRegistrar size= " << sizeof(meta::MetaClass::MetaExtensionRegistrar) << std::endl;
    std::cerr << "ExtendedObject::StaticMetaClass size= " << sizeof(*ExtendedObject::getStaticMetaClass()) << std::endl;
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

TEST_F(ObjectFactoryTest, registerObjectExtension)
{
    EXPECT_TRUE(m_factory->registerMetaClass<LambdaInvokable>());
}

TEST_F(ObjectFactoryTest, deepRegister)
{
    m_factory->registerMetaClass(Object::getStaticMetaClass());
    EXPECT_EQ(m_registrySize + 4u, std::distance(m_factory->begin(), m_factory->end()));
    EXPECT_NE(nullptr, m_factory->findMetaClass("TestObject"));
    EXPECT_NE(nullptr, m_factory->findMetaClass("PreObject"));
    EXPECT_NE(nullptr, m_factory->findMetaClass("Interface"));
    EXPECT_NE(nullptr, m_factory->findMetaClass("AbstractClass"));
    EXPECT_NE(nullptr, m_factory->findMetaClass("meta.Object"));
}

TEST_F(ObjectFactoryTest, registerStubMetaClassTheRightWay)
{
    auto lambda = [](){};
    using StubType = meta::Invokable<decltype(lambda), lambda>;
    m_factory->registerMetaClass(StubType::getStaticMetaClass());
    EXPECT_EQ(m_registrySize + 1u, std::distance(m_factory->begin(), m_factory->end()));
    EXPECT_NE(nullptr, m_factory->findMetaClass(StubType::getStaticMetaClass()->getName()));
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

TEST_F(ObjectFactoryTest, registerDynamicMetaclass)
{
    m_factory->registerMetaClass(DynamicObject::getExtendableMetaClass());
    EXPECT_NE(nullptr, m_factory->findMetaClass("DynamicExtendedObject"));
    EXPECT_EQ(nullptr, m_factory->findMetaClass("DynamicObject"));
    EXPECT_NE(nullptr, m_factory->findMetaClass("ExtendedObject"));

    m_factory->registerMetaClass<DynamicObject>();
    EXPECT_NE(nullptr, m_factory->findMetaClass("DynamicObject"));
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

    auto metaClass = m_factory->findMetaClass(Object::getStaticMetaClass()->getName());
    ASSERT_EQ(Object::getStaticMetaClass(), metaClass);
    ASSERT_NE(nullptr, metaClass->create("doing"));
    auto castedObject = metaClass->create<Object>("next");
    ASSERT_NE(nullptr, castedObject);
}

TEST_F(ObjectFactoryTest, testMetaClassCastedCreate)
{
    m_factory->registerMetaClass(Object::getStaticMetaClass());
    auto metaClass = m_factory->findMetaClass(Object::getStaticMetaClass()->getName());
    ASSERT_NE(nullptr, metaClass);
    auto castedObject = metaClass->create<Object>("next");
    EXPECT_NE(nullptr, castedObject);
}

TEST_F(ObjectFactoryTest, staticMetaExtension)
{    
    ASSERT_TRUE(ExtendedObject::getStaticMetaClass()->isSealed());
    m_factory->registerMetaClass(ExtendedObject::getStaticMetaClass());

    auto object = m_factory->create<ExtendedObject>("test");
    ASSERT_NE(nullptr, object);

    auto result = object->invoke("getName");
    ASSERT_TRUE(result);
    EXPECT_EQ("test", std::string_view(*result));
}

TEST_F(ObjectFactoryTest, dynamicMetaExtension)
{
    m_factory->registerMetaClass(DynamicObject::getStaticMetaClass());
    m_factory->registerMetaClass(DynamicObject::getExtendableMetaClass());

    auto dynamic = DynamicObject::getExtendableMetaClass();
    ASSERT_NE(nullptr, dynamic);
    EXPECT_FALSE(dynamic->isSealed());

    auto lambda = [](DynamicObject* self)
    {
        META_LOG_INFO(self->getName());
    };
    using MetaLambda = meta::Invokable<decltype(lambda), lambda>;
    dynamic->addMetaExtension(*MetaLambda::getStaticMetaClass());
    EXPECT_NE(nullptr, dynamic->findMetaExtension(MetaLambda::getStaticMetaClass()->getName()));
}

TEST_F(ObjectFactoryTest, dynamicExtensions)
{
    auto dynamic = DynamicObject::getExtendableMetaClass();

    auto lambda = [](meta::ObjectExtension* self)
    {
        META_LOG_INFO(self->getObject()->getName());
    };
    using MetaLambda = meta::Invokable<decltype(lambda), lambda>;
    dynamic->addMetaExtension(*MetaLambda::getStaticMetaClass());

    auto dynObject = dynamic->create<DynamicObject>("dynObject");
    ASSERT_NE(nullptr, dynObject);

    EXPECT_CALL(*m_mockPrinter, log("dynObject"));
    dynObject->invoke(MetaLambda::getStaticMetaClass()->getName());
}

TEST_F(ObjectFactoryTest, addRegisteredExtensionToDynamicMetaClass)
{
    ASSERT_TRUE(m_factory->registerMetaClass(ExtendObjectFunction::getStaticMetaClass()));
    auto dynamic = DynamicObject::getExtendableMetaClass();
    ASSERT_NE(nullptr, dynamic);

    ASSERT_TRUE(dynamic->tryAddExtension("extendObjects"));
    auto object = dynamic->create<DynamicObject>("test");

    EXPECT_CALL(*m_mockPrinter, log("extends test"));
    meta::invoke(object, "extendObjects");
}

TEST_F(ObjectFactoryTest, createThroughMetaClassAddsExtensionsFromAllLevels)
{
    m_factory->registerMetaClass<DynamicObject>();
    m_factory->registerMetaClass(DynamicObject::getExtendableMetaClass());

    // This creates an object using the sealed meta class.
    auto sealed = m_factory->create<DynamicObject>("DynamicObject", "sealed");
    ASSERT_NE(nullptr, sealed);
    EXPECT_TRUE(sealed->getDynamicMetaClass()->isSealed());

    auto unsealed = m_factory->create<DynamicObject>("DynamicExtendedObject", "unsealed");
    ASSERT_NE(nullptr, unsealed);
    EXPECT_FALSE(unsealed->getDynamicMetaClass()->isSealed());
}

TEST_F(ObjectFactoryTest, dynamicMetaClassCreatedInstancegetsMoreExtensions)
{
    m_factory->registerMetaClass<DynamicObject>();
    m_factory->registerMetaClass(DynamicObject::getExtendableMetaClass());

    auto unsealed = m_factory->create<DynamicObject>("DynamicExtendedObject", "unsealed");
    EXPECT_NE(nullptr, unsealed->findExtension("getName"));
}

TEST_F(MetaLibraryTest, findMetaExtension)
{
    auto metaClass = ExtendedObject::getStaticMetaClass();
    auto getName = metaClass->findMetaExtension("getName");
    ASSERT_NE(nullptr, getName);
}

TEST_F(MetaLibraryTest, enumerateExtensions)
{
    auto metaClass = ExtendedObject::getStaticMetaClass();

    for (auto it = metaClass->beginExtensions(), end = metaClass->endExtensions(); it != end; ++it)
    {
        META_LOG_INFO(it->second->getName());
    }
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
