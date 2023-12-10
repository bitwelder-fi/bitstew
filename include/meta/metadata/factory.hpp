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

#include <string_view>
#include <unordered_map>

namespace meta
{

class MetaClass;

/// Object factory. Keeps a registry of meta classes of an application.
class META_API ObjectFactory
{
    friend struct ObjectFactoryPrivate;
    using MetaClassMap = std::unordered_map<std::string_view, const MetaClass*>;
    MetaClassMap m_registry;

public:
    /// Factory registry iterator.
    using MetaClassIterator = MetaClassMap::const_iterator;

    /// Registers a meta class.
    /// \param metaClass The meta class to register.
    /// \return If the meta class gets registered with success, returns \e true, otherwise \e false.
    bool registerMetaClass(const MetaClass* metaClass);

    /// Overrides a meta class which is registered with the class name given as argument. The meta
    /// class must be registered for the function to succeed.
    /// \param metaClass The meta class which overrides the already registered meta class.
    /// \return If the meta class gets overridden with success, returns \e true, otherwise \e false.
    bool overrideMetaClass(const MetaClass* metaClass);

    /// Finds a meta class registered under \a className.
    /// \param className The name under which the metaclass is registered.
    /// \return The meta class, or \e nullptr, if no meta class with name was found.
    const MetaClass* findMetaClass(std::string_view className) const;

    /// Returns the front of the meta class registry.
    MetaClassIterator begin() const
    {
        return m_registry.begin();
    }
    /// Returns the end of the meta class registry.
    MetaClassIterator end() const
    {
        return m_registry.end();
    }
};

} // namespace meta

#endif // META_FACTORY_HPP
