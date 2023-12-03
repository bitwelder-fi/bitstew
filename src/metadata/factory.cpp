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

#include <meta/metadata/factory.hpp>
#include <assert.hpp>

namespace meta
{

bool ObjectFactory::registerMetaClass(std::string_view className, const MetaClass* metaClass)
{
    auto result = m_registry.insert(std::make_pair(className, metaClass));
    return result.second;
}

bool ObjectFactory::overrideMetaClass(std::string_view className, const MetaClass* metaClass)
{
    auto it = m_registry.find(className);
    if (it != m_registry.end())
    {
        it->second = metaClass;
        return true;
    }

    return false;
}

const MetaClass* ObjectFactory::findMetaClass(std::string_view className) const
{
    auto it = m_registry.find(className);
    return it != m_registry.end() ? it->second : nullptr;
}

}
