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

#include <meta/object_extensions/object_extension.hpp>
#include <meta/object.hpp>
#include <meta/log/trace.hpp>

namespace meta
{

Object::Object(std::string_view name) :
    MetaObject(name)
{
}

Object::~Object()
{
}

void Object::initialize()
{
    MetaObject::initialize();

    // Attach all un-attached extensions.
    for (auto it = m_extensions.begin(), end = m_extensions.end(); it != end; ++it)
    {
        if (!it->second->getObject())
        {
            it->second->attachToObject(*this);
        }
    }
}

ObjectPtr Object::create(std::string_view name)
{
    auto object = ObjectPtr(new Object(name));
    object->initialize();
    return object;
}

void Object::addExtension(ObjectExtensionPtr extension)
{
    if (extension->getObject().get() == this)
    {
        META_LOG_ERROR("Extension '" << extension->getName() <<"' already extends the object.");
        return;
    }

    auto it = m_extensions.insert(std::make_pair(extension->getName(), extension));
    if (it.second)
    {
        extension->attachToObject(*this);
    }
}

bool Object::removeExtension(ObjectExtension& extension)
{
    abortIfFail(extension.getObject().get() == this);

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

ObjectExtensionPtr Object::findExtension(std::string_view name) const
{
    auto it = m_extensions.find(name);
    return (it != m_extensions.end()) ? it->second : ObjectExtensionPtr();
}

ReturnValue Object::invoke(std::string_view name, PackagedArguments arguments)
{
    abortIfFail(!name.empty());

    auto extension = findExtension(name);
    if (!extension)
    {
        return std::nullopt;
    }
    return extension->run(arguments);
}


ReturnValue invoke(ObjectPtr object, std::string_view name, PackagedArguments arguments)
{
    abortIfFail(object && !name.empty());
    return object->invoke(name, arguments);
}

}
