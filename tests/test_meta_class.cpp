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

// {TMetaClass, bool-value}
template <typename TClass, bool value>
struct BoolParam
{
    const meta::MetaClass* metaClass = TClass::getStaticMetaClass();
    bool test = value;
};

template <typename TBaseClass, typename TDerivedClass, bool Result>
struct TwoTypesRelationTest
{
    using BaseClass = TBaseClass;
    using DerivedClass = TDerivedClass;

    const meta::MetaClass* baseClass = TBaseClass::getStaticMetaClass();
    const meta::MetaClass* derivedClass = TDerivedClass::getStaticMetaClass();
    bool test = Result;
};


template <typename Traits>
using MetaClassTestsGetStaticMetaClass = TypedMetaClassTests<Traits>;

using GetNameParam = MetaClassParam<std::string_view>;
using MetaClass_GetName = BaseParamMetaClassTest<std::string_view, MetaClassTests>;

using FindExtensionParam = MetaClassParam<std::vector<std::string_view>>;
using MetaClass_FindExtension = BaseParamMetaClassTest<std::vector<std::string_view>, MetaClassTests>;

template <typename Traits>
using MetaClassTestsIsAbstractMetaClass = BaseTypedMetaClassTests<Traits>;

template <typename Traits>
using MetaClassTestsIsDerivedFrom = BaseTypedMetaClassTests<Traits>;

}

INSTANTIATE_TEST_SUITE_P(MetaClassTests, MetaNameValidityTest,
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
                             MetaNameParam("meta*Object", true),
                             MetaNameParam("meta(Object", true),
                             MetaNameParam("meta)Object", true),
                             MetaNameParam("meta(*)Object", true),
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


using GetStaticMetaClassTypes = ::testing::Types<meta::MetaObject,
                                                 meta::Object,
                                                 meta::ExecutableExtension,
                                                 AbstractClass,
                                                 Interface,
                                                 OverrideClass,
                                                 PreObject,
                                                 Object,
                                                 ExtendedObject,
                                                 DynamicObject,
                                                 ExtendObjectFunction,
                                                 LambdaInvokable>;
TYPED_TEST_SUITE(MetaClassTestsGetStaticMetaClass, GetStaticMetaClassTypes);
TYPED_TEST(MetaClassTestsGetStaticMetaClass, getStaticMetaClass)
{
    using MetaClassType = typename TestFixture::Traits;
    auto metaClass = MetaClassType::getStaticMetaClass();
    ASSERT_NE(nullptr, metaClass);
}
TYPED_TEST(MetaClassTestsGetStaticMetaClass, isSealed)
{
    using MetaClassType = typename TestFixture::Traits;
    auto metaClass = MetaClassType::getStaticMetaClass();
    EXPECT_TRUE(metaClass->isSealed());
}
TYPED_TEST(MetaClassTestsGetStaticMetaClass, registerMetaClass)
{
    meta::ObjectFactory factory;
    using MetaClassType = typename TestFixture::Traits;
    auto metaClass = MetaClassType::getStaticMetaClass();

    EXPECT_TRUE(factory.registerMetaClass(metaClass));
    EXPECT_FALSE(factory.registerMetaClass(metaClass));
}
TYPED_TEST(MetaClassTestsGetStaticMetaClass, registerMetaClassWithTemplate)
{
    meta::ObjectFactory factory;
    using MetaClassType = typename TestFixture::Traits;

    EXPECT_TRUE(factory.registerMetaClass<MetaClassType>());
    EXPECT_FALSE(factory.registerMetaClass<MetaClassType>());
}

INSTANTIATE_TEST_SUITE_P(MetaClassTests, MetaClass_GetName,
                         ::testing::Values(
                             GetNameParam{meta::MetaObject::getStaticMetaClass(), "meta.MetaObject"},
                             GetNameParam{meta::Object::getStaticMetaClass(), "meta.Object"},
                             GetNameParam{AbstractClass::getStaticMetaClass(), "AbstractClass"},
                             GetNameParam{Interface::getStaticMetaClass(), "Interface"},
                             GetNameParam{OverrideClass::getStaticMetaClass(), "AbstractClass"},
                             GetNameParam{PreObject::getStaticMetaClass(), "PreObject"},
                             GetNameParam{Object::getStaticMetaClass(), "TestObject"},
                             GetNameParam{ExtendedObject::getStaticMetaClass(), "ExtendedObject"},
                             GetNameParam{DynamicObject::getStaticMetaClass(), "DynamicObject"},
                             GetNameParam{ExtendObjectFunction::getStaticMetaClass(), "extendObjects"},
                             GetNameParam{LambdaInvokable::getStaticMetaClass(), "lambda"}));
TEST_P(MetaClass_GetName, metaClass_getName)
{
    EXPECT_EQ(this->param.param, this->param.metaClass->getName());
}

using IsAbstractMetaClassTypes = ::testing::Types<BoolParam<meta::MetaObject, false>,
                                                  BoolParam<meta::Object, false>,
                                                  BoolParam<Interface, true>,
                                                  BoolParam<AbstractClass, true>,
                                                  BoolParam<Object, false>>;
TYPED_TEST_SUITE(MetaClassTestsIsAbstractMetaClass, IsAbstractMetaClassTypes);
TYPED_TEST(MetaClassTestsIsAbstractMetaClass, staticMetaClass_isAbstract)
{
    EXPECT_EQ(this->traits.test, this->traits.metaClass->isAbstract());
}

using IsDerivedFromTypes = ::testing::Types<TwoTypesRelationTest<meta::MetaObject, meta::MetaObject, false>,
                                            TwoTypesRelationTest<meta::Object, meta::MetaObject, false>,
                                            TwoTypesRelationTest<meta::MetaObject, meta::Object, true>,
                                            TwoTypesRelationTest<meta::MetaObject, Object, true>,
                                            TwoTypesRelationTest<Interface, AbstractClass, false>,
                                            TwoTypesRelationTest<meta::MetaObject, AbstractClass, true>,
                                            TwoTypesRelationTest<Interface, Object, true>>;
TYPED_TEST_SUITE(MetaClassTestsIsDerivedFrom, IsDerivedFromTypes);
TYPED_TEST(MetaClassTestsIsDerivedFrom, staticMetaClass_isDerivedFrom)
{
    using BaseClass = typename TestFixture::Traits::BaseClass;
    EXPECT_EQ(this->traits.test, this->traits.derivedClass->isDerivedFrom(*this->traits.baseClass));
    EXPECT_EQ(this->traits.test, this->traits.derivedClass->template isDerivedFromClass<BaseClass>());
}

INSTANTIATE_TEST_SUITE_P(MetaClassTests, MetaClass_FindExtension,
                         ::testing::Values(
                             FindExtensionParam{meta::MetaObject::getStaticMetaClass(), {}},
                             FindExtensionParam{meta::Object::getStaticMetaClass(), {}},
                             FindExtensionParam{AbstractClass::getStaticMetaClass(), {}},
                             FindExtensionParam{Interface::getStaticMetaClass(), {}},
                             FindExtensionParam{OverrideClass::getStaticMetaClass(), {}},
                             FindExtensionParam{PreObject::getStaticMetaClass(), {}},
                             FindExtensionParam{Object::getStaticMetaClass(), {}},
                             FindExtensionParam{ExtendedObject::getStaticMetaClass(), {"getName"}},
                             FindExtensionParam{DynamicObject::getStaticMetaClass(), {"getName"}}));
TEST_P(MetaClass_FindExtension, findMetaExtension)
{
    for (auto& extensionName : this->param.param)
    {
        auto extension = this->param.metaClass->findMetaExtension(extensionName);
        EXPECT_NE(nullptr, extension);
    }
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

TEST_F(ObjectFactoryTest, registerAutoMetaClass)
{
    auto lambda = [](){};
    using AutoType = meta::Invokable<decltype(lambda), lambda>;
    m_factory->registerMetaClass(AutoType::getStaticMetaClass());
    EXPECT_EQ(m_registrySize + 1u, std::distance(m_factory->begin(), m_factory->end()));
    EXPECT_NE(nullptr, m_factory->findMetaClass(AutoType::getStaticMetaClass()->getName()));
}

TEST_F(ObjectFactoryTest, registerOverride)
{
    EXPECT_TRUE(m_factory->registerMetaClass(AbstractClass::getStaticMetaClass()));
    EXPECT_TRUE(m_factory->overrideMetaClass(OverrideClass::getStaticMetaClass()));
}

TEST_F(ObjectFactoryTest, deepRegisterOverride)
{
    EXPECT_TRUE(m_factory->registerMetaClass(AbstractClass::getStaticMetaClass()));
    EXPECT_EQ(m_registrySize + 1u, std::distance(m_factory->begin(), m_factory->end()));
    EXPECT_NE(nullptr, m_factory->findMetaClass("AbstractClass"));
    EXPECT_NE(nullptr, m_factory->findMetaClass("meta.Object"));
    EXPECT_EQ(nullptr, m_factory->findMetaClass("Interface"));

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

TEST_F(ObjectFactoryTest, findMetaClass)
{
    registerTestClasses();

    EXPECT_EQ(Interface::getStaticMetaClass(), m_factory->findMetaClass("Interface"));
}

TEST_F(ObjectFactoryTest, create)
{
    registerTestClasses();

    ASSERT_NE(nullptr, m_factory->create("TestObject", "doing"));
}

TEST_F(ObjectFactoryTest, create_template)
{
    registerTestClasses();

    ASSERT_NE(nullptr, m_factory->create<Object>("doing"));
}

TEST_F(ObjectFactoryTest, createWithMetaExtension)
{    
    registerTestClasses();

    auto object = m_factory->create<ExtendedObject>("test");
    ASSERT_NE(nullptr, object);

    auto result = object->invoke("getName");
    ASSERT_TRUE(result);
    EXPECT_EQ("test", std::string_view(*result));
}

TEST_F(ObjectFactoryTest, createWithMetaExtensionInSuper)
{
    registerTestClasses();

    auto object = m_factory->create<DynamicObject>("test");
    ASSERT_NE(nullptr, object);

    auto result = object->invoke("getName");
    ASSERT_TRUE(result);
    EXPECT_EQ("test", std::string_view(*result));
}

TEST_F(ObjectFactoryTest, addDynamicMetaExtensionToMetaClass)
{
    registerTestClasses();

    auto dynamic = DynamicObject::getExtendableMetaClass();
    ASSERT_NE(nullptr, dynamic);
    EXPECT_FALSE(dynamic->isSealed());

    dynamic->addMetaExtension(LambdaInvokable::getStaticMetaClass());
    EXPECT_NE(nullptr, dynamic->findMetaExtension("lambda"));
    EXPECT_NE(nullptr, dynamic->findMetaExtension("getName"));
}

TEST_F(ObjectFactoryTest, createWithoutFactoryIsMissingMetaExtensions)
{
    auto object = DynamicObject::create("test");

    auto result = meta::invoke(object, "getName");
    EXPECT_EQ(std::nullopt, result);
}


TEST_F(MetaLibraryTest, metaLibrarysHasObjectFactory)
{
    auto factory = meta::Library::instance().objectFactory();
    EXPECT_NE(nullptr, factory);
}

TEST_F(MetaLibraryTest, metaLibraryObjectFactoryRegistryDefaultContent)
{
    auto factory = meta::Library::instance().objectFactory();
    EXPECT_EQ(3u, std::distance(factory->begin(), factory->end()));

    EXPECT_NE(nullptr, factory->findMetaClass("meta.MetaObject"));
    EXPECT_NE(nullptr, factory->findMetaClass("meta.Object"));
    EXPECT_NE(nullptr, factory->findMetaClass("meta.ExecutableExtension"));
}
