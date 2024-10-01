/*
 * Copyright (C) 2024 bitWelder
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

#include <stew/core/assert.hpp>
#include <stew/dynamic_type/exceptions.hpp>
#include <stew/dynamic_type/type_converter.hpp>
#include <stew/dynamic_type/type_registry.hpp>

namespace stew
{


TypeRegistry& TypeRegistry::instance()
{
    static TypeRegistry registry;

    if (!registry.m_isInitialized)
    {
        registry.initialize();
    }

    return registry;
}

void TypeRegistry::uninitialize()
{
    m_converters.clear();
    m_types.clear();
    m_isInitialized = false;
}

void TypeRegistry::registerType(const TypeInfo& type, TypeOperators operators)
{
    const auto tindex = type.index();
    auto it = m_types.find(tindex);
    abortIfFailWithMessage(it == m_types.end(), "Type " << type.getName() << " already registered.");

    m_types.insert(std::make_pair(tindex, std::move(operators)));
}

TypeOperators& TypeRegistry::getTypeOperators(const TypeInfo& type)
{
    if (auto it = m_types.find(type.index()); it != m_types.end())
    {
        return it->second;
    }

    throw UnregisteredType(type);
}

TypeOperators* TypeRegistry::findTypeOperators(const TypeInfo& type)
{
    auto it = m_types.find(type.index());
    if (it == m_types.end())
    {
        return {};
    }

    return &it->second;
}

void TypeRegistry::registerTypeConverter(const TypeInfo& type, TypeConverter converter)
{
    const auto sourceIndex = std::type_index(type);
    auto t = m_types.find(sourceIndex);
    abortIfFailWithMessage(t != m_types.end(), "Type " << type.getName() << " is not registered.");

    const auto target = converter.target();
    const auto targetIndex = std::type_index(target);

    if (auto it = m_converters.find(sourceIndex); it == m_converters.end())
    {
        m_converters.insert(std::make_pair(sourceIndex, ConverterMap({std::make_pair(targetIndex, std::move(converter))})));
    }
    else
    {
        auto cit = it->second.find(targetIndex);
        abortIfFailWithMessage(cit == it->second.end(), "Converter " << type.getName() << " -> " << target.getName() << " already registered.");
        it->second.insert(std::make_pair(targetIndex, std::move(converter)));
    }
}

TypeConverter* TypeRegistry::findConverter(const TypeInfo& source, const TypeInfo& target)
{
    const auto sourceIndex = std::type_index(source);
    if (auto it = m_converters.find(sourceIndex); it != m_converters.end())
    {
        const auto targetIndex = std::type_index(target);
        if (auto cit = it->second.find(targetIndex); cit != it->second.end())
        {
            return &cit->second;
        }
    }

    return {};
}

}
