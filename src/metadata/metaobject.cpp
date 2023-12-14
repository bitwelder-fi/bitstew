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

#include <meta/metadata/metaobject.hpp>

namespace meta
{

MetaObject::MetaObject(std::string_view name) :
    m_name(name)
{
}

MetaObjectPtr MetaObject::create(std::string_view name)
{
    return MetaObjectPtr(new MetaObject(name));
}


std::optional<ArgumentData> invoke(MetaObjectPtr object, std::string_view invokableName, const PackagedArguments& arguments)
{
    abortIfFail(object && !invokableName.empty());

    auto metaClass = object->getStaticMetaClass();
    auto metaMethod = metaClass->findMethod(invokableName);
    if (!metaMethod)
    {
        return std::nullopt;
    }
    return metaMethod->apply(object.get(), arguments);
}

}
