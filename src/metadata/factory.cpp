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
#include <stew/log/trace.hpp>
#include <stew/metadata/factory.hpp>
#include <stew/metadata/metaclass.hpp>
#include <stew/metadata/meta_object.hpp>

namespace stew
{

bool ObjectFactory::registerMetaClass(const MetaClass* metaClass)
{
    abortIfFail(metaClass);
    if (metaClass->getName().empty())
    {
        STEW_LOG_ERROR("Attempt registering stub meta-class.");
        return false;
    }
    if (!isValidMetaName(metaClass->getName()))
    {
        STEW_LOG_ERROR("Invalid meta-class name: " << metaClass->getName());
        return false;
    }

    // Deep register also any unregistered super class.
    auto result = false;
    auto deepRegister = [this, &result](auto metaClass)
    {
        auto it = m_registry.insert(std::make_pair(metaClass->getName(), metaClass));
        result |= it.second;
        return MetaClass::VisitResult::Continue;
    };
    metaClass->visit(deepRegister);

    return result;
}

bool ObjectFactory::overrideMetaClass(const MetaClass* metaClass)
{
    abortIfFail(metaClass);
    auto it = m_registry.find(metaClass->getName());
    if (it != m_registry.end())
    {
        it->second = metaClass;
        auto deepRegister = [this](auto super)
        {
            m_registry.insert(std::make_pair(super->getName(), super));
            return MetaClass::VisitResult::Continue;
        };
        metaClass->visitSuper(deepRegister);
        return true;
    }

    return false;
}

const MetaClass* ObjectFactory::findMetaClass(std::string_view className) const
{
    if (!isValidMetaName(className))
    {
        STEW_LOG_ERROR("Invalid meta-class name: " << className);
        return {};
    }
    auto it = m_registry.find(className);
    return it != m_registry.end() ? it->second : nullptr;
}

MetaObjectPtr ObjectFactory::create(std::string_view className, std::string_view instanceName)
{
    auto metaClass = findMetaClass(className);
    if (!metaClass)
    {
        return {};
    }
    return metaClass->create(instanceName);
}

}
