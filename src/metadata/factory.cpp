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

#include <assert.hpp>
#include <meta/log/trace.hpp>
#include <meta/metadata/factory.hpp>
#include <meta/metadata/metaclass.hpp>
#include <meta/metadata/meta_object.hpp>

namespace meta
{

struct ObjectFactoryPrivate
{
    static void deepRegister(ObjectFactory& self, const MetaClass* metaClass)
    {
        for (std::size_t i = 0u; i < metaClass->getBaseClassCount(); ++i)
        {
            const auto baseClass = metaClass->getBaseClass(i);
            // Check if the base class is registered
            if (!self.findMetaClass(baseClass->getName()))
            {
                // registering will do deep register of base metaclasses.
                self.registerMetaClass(baseClass);
            }
        }
    }

    static void deepOverride(ObjectFactory& self, const MetaClass* metaClass)
    {
        for (std::size_t i = 0u; i < metaClass->getBaseClassCount(); ++i)
        {
            const auto baseClass = metaClass->getBaseClass(i);
            // Check if the base class is registered. If it is, override with the new metaclass.
            // If it is not yet registered, register.
            auto it = self.m_registry.find(baseClass->getName());
            if (it != self.m_registry.end())
            {
                it->second = metaClass;
            }
            else
            {
                // Register this metaclass only. Deep register may cause failure.
                self.m_registry.insert(std::make_pair(baseClass->getName(), metaClass));
            }
            deepOverride(self, baseClass);
        }
    }
};

bool ObjectFactory::registerMetaClass(const MetaClass* metaClass)
{
    abortIfFail(metaClass);
    if (metaClass->getName().empty())
    {
        META_LOG_ERROR("Attempt registering stub meta class.");
        return false;
    }
    if (!isValidMetaName(metaClass->getName()))
    {
        META_LOG_ERROR("Invalid meta class name: " << metaClass->getName());
        return false;
    }
    auto result = m_registry.insert(std::make_pair(metaClass->getName(), metaClass));
    ObjectFactoryPrivate::deepRegister(*this, metaClass);
    return result.second;
}

bool ObjectFactory::overrideMetaClass(const MetaClass* metaClass)
{
    abortIfFail(metaClass);
    auto it = m_registry.find(metaClass->getName());
    if (it != m_registry.end())
    {
        it->second = metaClass;
        ObjectFactoryPrivate::deepOverride(*this, metaClass);
        return true;
    }

    return false;
}

const MetaClass* ObjectFactory::findMetaClass(std::string_view className) const
{
    if (!isValidMetaName(className))
    {
        META_LOG_ERROR("Invalid meta class name: " << className);
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
