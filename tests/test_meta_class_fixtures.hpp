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

#ifndef META_TEST_META_CLASS_FIXTURES_HPP
#define META_TEST_META_CLASS_FIXTURES_HPP

#include <gtest/gtest.h>

#include <meta/meta.hpp>
#include <meta/metadata/factory.hpp>
#include <meta/metadata/metaclass.hpp>
#include <meta/library_config.hpp>
#include <meta/object.hpp>
#include <meta/object_extensions/invokable.hpp>
#include <utils/scope_value.hpp>

#include "utils/domain_test_environment.hpp"


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

    STATIC_META_CLASS("Interface", Interface)
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

    META_CLASS("TestObject", Object, PreObject, Interface)
    {
    };

    static std::shared_ptr<Object> create(std::string_view name)
    {
        auto object = std::shared_ptr<Object>(new Object(name));
        object->initialize();
        return object;
    }

protected:
    explicit Object(std::string_view name) :
        PreObject(name)
    {
    }
};

class ExtendedObject : public Object
{
public:
    DECLARE_INVOKABLE(MetaGetName, "getName", &ExtendedObject::getName);
    META_CLASS("ExtendedObject", ExtendedObject, Object)
    {
        META_EXTENSION(MetaGetName);
    };

    static std::shared_ptr<ExtendedObject> create(std::string_view name)
    {
        auto object = std::shared_ptr<ExtendedObject>(new ExtendedObject(name));
        object->initialize();
        return object;
    }

protected:
    explicit ExtendedObject(std::string_view name) :
        Object(name)
    {
    }
};

class DynamicObject : public ExtendedObject
{
public:
    STATIC_META_CLASS("DynamicObject", DynamicObject, ExtendedObject)
    {
    };

    class DynamicExtendedObject : public DynamicObject::StaticMetaClass
    {
    public:
        explicit DynamicExtendedObject(std::string_view className) :
            StaticMetaClass(className)
        {
            m_descriptor->sealed = false;
        }
    };
    const meta::MetaClass* getDynamicMetaClass() const override
    {
        auto metaClass = getFactory();
        if (!metaClass)
        {
            metaClass = getStaticMetaClass();
        }
        return metaClass;
    }

    static meta::MetaClass* getExtendableMetaClass()
    {
        static DynamicExtendedObject dynamicClass("DynamicExtendedObject");
        return &dynamicClass;
    }

    static auto create(std::string_view name)
    {
        auto object = std::shared_ptr<DynamicObject>(new DynamicObject(name));
        object->initialize();
        return object;
    }

protected:
    explicit DynamicObject(std::string_view name) :
        ExtendedObject(name)
    {
    }
};

void extendObjects(meta::ObjectExtension* self)
{
    META_LOG_INFO("extends " << self->getObject()->getName());
}
DECLARE_INVOKABLE(ExtendObjectFunction, "extendObjects", &extendObjects);

auto globalLambda = [](meta::ObjectExtension* self)
{
    META_LOG_INFO(self->getObject()->getName());
};
DECLARE_INVOKABLE(LambdaInvokable, "lambda", globalLambda);


// -------------------------------------------------------------------------------------------------
// Test fixtures.
struct MetaClassTestHelpers
{
    std::size_t getBaseClassCount(const meta::MetaClass* metaClass) const
    {
        auto count = std::size_t(0);
        auto visitor = [&count](auto)
        {
            ++count;
            return meta::MetaClass::VisitResult::Continue;
        };
        metaClass->visitSuper(visitor);
        return count;
    }
};

class MetaClassTests : public ::testing::Test, public MetaClassTestHelpers
{
};

template <typename TestTraits>
class TypedMetaClassTests : public MetaClassTests
{
protected:
    using Traits = TestTraits;
};

template <typename TestTraits>
class BaseTypedMetaClassTests : public TypedMetaClassTests<TestTraits>
{
protected:
    TestTraits traits;
};


// Base Test fixture with a meta-class and a parameter with arbitrar type.
template <typename Param>
struct MetaClassParam
{
    const meta::MetaClass* metaClass;
    Param param;
};
template <typename Param, typename TestBase>
class BaseParamMetaClassTest : public TestBase, public ::testing::WithParamInterface<MetaClassParam<Param>>
{
public:
    using ParamType = MetaClassParam<Param>;

protected:
    ParamType param;
    void SetUp() override
    {
        TestBase::SetUp();
        param = this->GetParam();
    }
};

class ObjectFactoryTest : public DomainTestEnvironment, public MetaClassTestHelpers
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

    void registerTestClasses()
    {
        m_factory->registerMetaClass<AbstractClass>();
        m_factory->registerMetaClass<Interface>();
        m_factory->registerMetaClass<PreObject>();
        m_factory->registerMetaClass<Object>();
        m_factory->registerMetaClass<ExtendedObject>();
        m_factory->registerMetaClass<DynamicObject>();
        m_factory->registerMetaClass(DynamicObject::getExtendableMetaClass());

        m_factory->registerMetaClass<ExtendObjectFunction>();
        m_factory->registerMetaClass<LambdaInvokable>();
    }
};

using MetaNameParam = std::tuple<std::string, bool>;
class MetaNameValidityTest : public ::testing::Test,
                             public MetaClassTestHelpers,
                             public ::testing::WithParamInterface<MetaNameParam>
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

class MetaLibraryTest : public DomainTestEnvironment, public MetaClassTestHelpers
{
protected:
    void SetUp() override
    {
        DomainTestEnvironment::initializeDomain(true, true);
    }
};

#endif // META_TEST_META_CLASS_FIXTURES_HPP
