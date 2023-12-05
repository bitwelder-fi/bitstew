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
