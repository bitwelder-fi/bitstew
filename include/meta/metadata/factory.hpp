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

#ifndef META_FACTORY_HPP
#define META_FACTORY_HPP

#include <meta/meta_api.hpp>
#include <meta/metadata/metaclass.hpp>

#include <string_view>
#include <unordered_map>

namespace meta
{

class MetaClass;

/// Object factory. Keeps a registry of meta classes of an application.
class META_API ObjectFactory
{
    using MetaClassMap = std::unordered_map<std::string_view, const MetaClass*>;
    MetaClassMap m_registry;

public:
    /// Factory registry iterator.
    using MetaClassIterator = MetaClassMap::const_iterator;

    /// Registers a meta-class.
    /// \param metaClass The meta-class to register.
    /// \return If the meta-class gets registered with success, returns \e true, otherwise \e false.
    bool registerMetaClass(const MetaClass* metaClass);

    template <class ClassType>
    bool registerMetaClass()
    {
        return registerMetaClass(ClassType::getStaticMetaClass());
    }

    /// Overrides a meta-class registered under the same name as the overriding metaclass.
    /// \param metaClass The meta-class which overrides the previously registered meta-class.
    /// \return If the meta-class gets overridden with success, returns \e true, otherwise \e false.
    bool overrideMetaClass(const MetaClass* metaClass);

    /// Finds a meta-class registered under \a className.
    /// \param className The name under which the metaclass is registered.
    /// \return The meta-class, or \e nullptr, if no meta-class with name was found.
    const MetaClass* findMetaClass(std::string_view className) const;

    /// Returns the front of the meta-class registry.
    MetaClassIterator begin() const
    {
        return m_registry.begin();
    }
    /// Returns the end of the meta-class registry.
    MetaClassIterator end() const
    {
        return m_registry.end();
    }

    /// Creates an instance from the registered meta-class of an ObjectType
    /// \tparam ObjectType The object type whose meta-class must be registered.
    /// \param instanceName The name with which to create the instance.
    /// \return On success, returns the instance created, or an invalid shared pointer on failure.
    template <class ObjectType>
    std::shared_ptr<ObjectType> create(std::string_view instanceName)
    {
        return std::dynamic_pointer_cast<ObjectType>(create(ObjectType::getStaticMetaClass()->getName(), instanceName));
    }

    /// Creates an instance from the registered meta-class identified by the className, with the passed Arguments.
    /// \param className The name of the meta-class.
    /// \param instanceName The name with which to create the instance.
    /// \return On success, returns the instance created, or an invalid shared pointer on failure.
    MetaObjectPtr create(std::string_view className, std::string_view instanceName);

    template <class ObjectType>
    std::shared_ptr<ObjectType> create(std::string_view className, std::string_view instanceName)
    {
        return std::dynamic_pointer_cast<ObjectType>(create(className, instanceName));
    }
};

} // namespace meta

#endif // META_FACTORY_HPP
