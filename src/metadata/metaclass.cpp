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

#include <meta/meta.hpp>
#include <meta/metadata/factory.hpp>
#include <meta/metadata/metaclass.hpp>
#include <meta/object.hpp>
#include <meta/object_extensions/data_extension.hpp>
#include <meta/object_extensions/executable_extension.hpp>
#include <utils/scope_value.hpp>

namespace meta
{

MetaObject::MetaObject(std::string_view metaName) :
    m_name(metaName)
{
    abortIfFail(isValidMetaName(m_name));
}

MetaObject::~MetaObject()
{
}


Registrars::Extension::Extension(Registrars& self, const MetaClass* extension)
{
    auto registrar = [extension](MetaClass& metaClass)
    {
        metaClass.addMetaExtension(extension);
    };
    self.m_registrars.push_back(registrar);
}

void Registrars::apply(MetaClass& metaClass)
{
    for (auto& registrar : m_registrars)
    {
        registrar(metaClass);
    }
}


MetaObjectPtr MetaClass::create(std::string_view name) const
{
    abortIfFail(m_descriptor);
    auto object = m_descriptor->create(name);
    if (object)
    {
        object->m_factory = const_cast<MetaClass*>(this);
    }
    auto extendable = std::dynamic_pointer_cast<Object>(object);
    if (extendable)
    {
        initializeInstance(extendable);
    }
    return object;
}

void MetaClass::initializeInstance(ObjectPtr instance) const
{
    auto visitor = [instance](auto metaClass)
    {
        for (auto& metaExtension : metaClass->m_descriptor->extensions)
        {
            if (metaExtension.second->template isDerivedFromClass<ExecutableExtension>())
            {
                auto extension = metaExtension.second->template create<ExecutableExtension>(metaExtension.second->getName());
                instance->addExtension(extension);
            }
            if (metaExtension.second->template isDerivedFromClass<DataExtension>())
            {
                auto data = metaExtension.second->template create<DataExtension>(metaExtension.second->getName());
                instance->addData(data);
            }
        }
        return VisitResult::Continue;
    };
    visit(visitor);
}

bool MetaClass::isSealed() const
{
    abortIfFail(m_descriptor);
    return m_descriptor->sealed;
}

std::string_view MetaClass::getName() const
{
    abortIfFail(m_descriptor);
    return m_descriptor->name;
}

bool MetaClass::isAbstract() const
{
    abortIfFail(m_descriptor);
    return m_descriptor->isAbstract();
}

bool MetaClass::isDerivedFrom(const MetaClass& metaClass) const
{
    abortIfFail(m_descriptor);

    auto visitor = [&metaClass](auto super)
    {
        return (super == &metaClass) ? VisitResult::Abort : VisitResult::Continue;
    };
    return visitSuper(visitor) == VisitResult::Abort;
}

MetaClass::VisitResult MetaClass::visit(Visitor visitor) const
{
    auto result = visitor(this);
    if (result == VisitResult::Abort)
    {
        return result;
    }
    return visitSuper(visitor);
}

MetaClass::VisitResult MetaClass::visitSuper(Visitor visitor) const
{
    return m_descriptor->visitSuper(visitor);
}

void MetaClass::addMetaExtension(const MetaClass* extensionMeta)
{
    abortIfFail(m_descriptor && extensionMeta && !m_descriptor->sealed && extensionMeta->m_descriptor && extensionMeta->m_descriptor->isExtension());

    // The metaExtension must have a name.
    abortIfFail(!extensionMeta->getName().empty());
    auto result = m_descriptor->extensions.insert({extensionMeta->getName(), extensionMeta});
    abortIfFail(result.second);
}

bool MetaClass::tryAddExtension(std::string_view metaName)
{
    abortIfFail(m_descriptor && isValidMetaName(metaName));

    auto metaClass = Library::instance().objectFactory()->findMetaClass(metaName);
    if (!metaClass)
    {
        return false;
    }
    if (!metaClass->m_descriptor->isExtension())
    {
        return false;
    }

    auto result = m_descriptor->extensions.insert({metaName, metaClass});
    return result.second;
}

const MetaClass* MetaClass::findMetaExtension(std::string_view name) const
{
    abortIfFail(isValidMetaName(name) && m_descriptor);

    const MetaClass* result = nullptr;
    auto visitor = [&result, name](auto metaClass)
    {
        auto it = metaClass->m_descriptor->extensions.find(name);
        if (it != metaClass->m_descriptor->extensions.end())
        {
            result = it->second;
            return VisitResult::Abort;
        }
        return VisitResult::Continue;
    };
    visit(visitor);

    return result;
}

MetaClass::MetaExtensionIterator MetaClass::beginExtensions() const
{
    return m_descriptor->extensions.begin();
}

MetaClass::MetaExtensionIterator MetaClass::endExtensions() const
{
    return m_descriptor->extensions.end();
}

}
