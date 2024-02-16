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

#ifndef META_TEST_META_CLASS_FICXTURES_HPP
#define META_TEST_META_CLASS_FICXTURES_HPP

#include <gtest/gtest.h>

#include <meta/meta.hpp>
#include <meta/metadata/factory.hpp>
#include <meta/metadata/metaclass.hpp>
#include <meta/library_config.hpp>
#include <meta/object.hpp>
#include <meta/object_extensions/invokable.hpp>
#include <utils/scope_value.hpp>


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
        return std::shared_ptr<Object>(new Object(name));
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

    static std::shared_ptr<Object> create(std::string_view name)
    {
        return std::shared_ptr<Object>(new ExtendedObject(name));
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
        return std::shared_ptr<Object>(new DynamicObject(name));
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

auto globalLambda = [](){};
DECLARE_INVOKABLE(LambdaInvokable, "lambda", globalLambda);

#endif // META_TEST_META_CLASS_FICXTURES_HPP
