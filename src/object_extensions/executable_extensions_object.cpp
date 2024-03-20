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

#include <meta/log/trace.hpp>
#include <meta/object_extensions/executable_extensions_object.hpp>
#include <meta/object_extensions/executable_extension.hpp>

#include <string_view>

namespace meta
{

ExecutableExtensionsObject::~ExecutableExtensionsObject()
{
    // Detach all attached extensions.
    for (auto it = m_extensions.begin(), end = m_extensions.end(); it != end; ++it)
    {
        if (it->second)
        {
            it->second->detachFromObject();
        }
    }
}

void ExecutableExtensionsObject::initialize()
{
    // Attach all un-attached extensions.
    for (auto it = m_extensions.begin(), end = m_extensions.end(); it != end; ++it)
    {
        if (!it->second->getObject())
        {
            it->second->attachToObject(*this);
        }
    }
}

void ExecutableExtensionsObject::addExtension(ExecutableExtensionPtr extension)
{
    if (extension->getObject() == this)
    {
        META_LOG_ERROR("Extension '" << extension->getName() << "' already extends the object.");
        return;
    }

    auto it = m_extensions.insert(std::make_pair(extension->getName(), extension));
    if (it.second)
    {
        extension->attachToObject(*this);
    }
}

bool ExecutableExtensionsObject::removeExtension(ExecutableExtension& extension)
{
    abortIfFail(extension.getObject() == this);

    auto it = m_extensions.find(extension.getName());
    if (it != m_extensions.end())
    {
        extension.detachFromObject();
        m_extensions.erase(it);
        return true;
    }
    META_LOG_ERROR("Extension " << extension.getName() <<" does not extend the object.");
    return false;
}

ExecutableExtensionPtr ExecutableExtensionsObject::findExtension(std::string_view name) const
{
    auto it = m_extensions.find(name);
    return (it != m_extensions.end()) ? it->second : ExecutableExtensionPtr();
}

ReturnValue ExecutableExtensionsObject::invoke(std::string_view name, PackagedArguments arguments)
{
    abortIfFail(!name.empty());

    auto extension = findExtension(name);
    if (!extension)
    {
        return std::nullopt;
    }
    return extension->run(arguments);
}


ReturnValue invoke(ExecutableExtensionsObject* object, std::string_view name, PackagedArguments arguments)
{
    abortIfFail(object && !name.empty());
    return object->invoke(name, arguments);
}

ReturnValue invoke(ExecutableExtensionsObjectPtr object, std::string_view name, PackagedArguments arguments)
{
    return invoke(object.get(), name, arguments);
}

}
