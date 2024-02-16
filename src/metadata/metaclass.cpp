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
#include <meta/object_extensions/object_extension.hpp>
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


MetaClass::MetaExtensionRegistrar::MetaExtensionRegistrar(MetaClass& self, const MetaClass& extensionMeta)
{
    utils::ScopeValue<bool> unlock(self.m_descriptor->sealed, false);
    self.addMetaExtension(extensionMeta);
}

MetaClass::MetaName::MetaName(MetaClass& self, std::string_view name)
{
    self.m_descriptor->name = ensureValidMetaName(std::string(name));
    abortIfFail(isValidMetaName(self.m_descriptor->name));
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
    for (auto& metaExtension : m_descriptor->extensions)
    {
        auto extension = metaExtension.second->create<ObjectExtension>(metaExtension.second->getName());
        instance->addExtension(extension);
    }
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

const MetaClass* MetaClass::getBaseClass(std::size_t index) const
{
    abortIfFail(m_descriptor);
    return m_descriptor->getBaseClass(index);
}

std::size_t MetaClass::getBaseClassCount() const
{
    abortIfFail(m_descriptor);
    return m_descriptor->getBaseClassCount();
}

bool MetaClass::isAbstract() const
{
    abortIfFail(m_descriptor);
    return m_descriptor->isAbstract();
}

bool MetaClass::isMetaClassOf(const MetaObject& object) const
{
    abortIfFail(m_descriptor);
    return m_descriptor->isMetaClassOf(object);
}

bool MetaClass::isDerivedFrom(const MetaClass& metaClass) const
{
    abortIfFail(m_descriptor);
    if (&metaClass == this)
    {
        return true;
    }
    return m_descriptor->hasSuperClass(metaClass);
}

void MetaClass::addMetaExtension(const MetaClass& extensionMeta)
{
    abortIfFail(m_descriptor && !m_descriptor->sealed && extensionMeta.m_descriptor && extensionMeta.m_descriptor->isExtension());

    // The metaExtension must have a name.
    abortIfFail(!extensionMeta.getName().empty());
    auto result = m_descriptor->extensions.insert({extensionMeta.getName(), &extensionMeta});
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
    auto it = m_descriptor->extensions.find(name);
    return it != m_descriptor->extensions.end() ? it->second : nullptr;
}

}
