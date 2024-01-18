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

#include <meta/metadata/invokable.hpp>
#include <meta/metadata/metaclass.hpp>

namespace meta
{

ObjectPtr MetaClass::create(std::string_view name) const
{
    abortIfFail(m_descriptor);
    return m_descriptor->create(name);
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

bool MetaClass::isMetaClassOf(const Object& object) const
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

bool MetaClass::addInvokable(MetaInvokable& invokable)
{
    abortIfFail(m_descriptor && !m_descriptor->sealed);
    auto result = m_descriptor->callables.insert({std::string(invokable.getName()), &invokable});
    if (!result.second)
    {
        META_LOG_ERROR("Invokable " << invokable.getName() <<" is already registered to metaclass.");
    }
    return result.second;
}

Invokable* MetaClass::findInvokable(std::string_view name) const
{
    abortIfFail(m_descriptor);
    auto it = m_descriptor->callables.find(std::string(name));
    if (it == m_descriptor->callables.end())
    {
        return {};
    }
    return it->second;
}

void MetaClass::removeInvokable(MetaInvokable& invokable)
{
    abortIfFail(m_descriptor);
    auto it = m_descriptor->callables.find(std::string(invokable.getName()));
    if (it == m_descriptor->callables.end())
    {
        META_LOG_ERROR("Invokable " << invokable.getName() <<" is not registered to metaclass.");
    }
    else
    {
        m_descriptor->callables.erase(it);
    }
}

}
