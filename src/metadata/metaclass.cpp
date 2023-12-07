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

#include <meta/metadata/callable.hpp>
#include <meta/metadata/metaclass_impl.hpp>

namespace meta
{

MetaObjectPtr Metaclass::create(std::string_view name) const
{
    abortIfFail(m_descriptor);
    return m_descriptor->create(name);
}

const Metaclass* Metaclass::getBaseClass(size_t index) const
{
    abortIfFail(m_descriptor);
    return m_descriptor->getBaseClass(index);
}

size_t Metaclass::getBaseClassCount() const
{
    abortIfFail(m_descriptor);
    return m_descriptor->getBaseClassCount();
}

bool Metaclass::isAbstract() const
{
    abortIfFail(m_descriptor);
    return m_descriptor->isAbstract();
}

bool Metaclass::isMetaClassOf(const MetaObject& object) const
{
    abortIfFail(m_descriptor);
    return m_descriptor->isMetaClassOf(object);
}

bool Metaclass::isDerivedFrom(const Metaclass& metaClass) const
{
    abortIfFail(m_descriptor);
    if (&metaClass == this)
    {
        return true;
    }
    return m_descriptor->hasSuperClass(metaClass);
}

bool Metaclass::addMethod(Callable& callable)
{
    abortIfFail(m_descriptor && !m_descriptor->sealed);
    auto result = m_descriptor->callables.insert({callable.getName(), &callable});
    if (!result.second)
    {
        META_LOG_ERROR("Callable " << callable.getName() <<" is already registered to metaclass.");
    }
    return result.second;
}

Callable* Metaclass::findMethod(std::string_view name)
{
    abortIfFail(m_descriptor);
    auto it = m_descriptor->callables.find(name);
    return it != m_descriptor->callables.end() ? it->second : nullptr;
}






MetaClass::~MetaClass()
{
}

bool MetaClass::isDerivedFrom(const MetaClass& metaClass) const
{
    if (&metaClass == this)
    {
        return true;
    }
    return hasSuperClass(metaClass);
}

bool MetaClass::addMethod(Callable& callable)
{
    auto result = m_callables.insert({callable.getName(), &callable});
    if (!result.second)
    {
        META_LOG_ERROR("Callable " << callable.getName() <<" is already registered to metaclass.");
    }
    return result.second;
}

Callable* MetaClass::findMethod(std::string_view name)
{
    auto it = m_callables.find(name);
    return it != m_callables.end() ? it->second : nullptr;
}

}
