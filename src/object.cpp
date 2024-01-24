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

#include <meta/object_extension.hpp>
#include <meta/object.hpp>
#include "private/object.hpp"

namespace meta
{

ObjectDescriptor::ObjectDescriptor(Object& object) :
    p_ptr(&object)
{
}

bool ObjectDescriptor::addExtention(ObjectExtensionPtr extension)
{
    abortIfFail(!extension->getOwner());

    auto it = extensions.insert(std::make_pair(extension->getName(), extension));
    if (it.second)
    {
        it.first->second->m_descriptor->owner = p_ptr->shared_from_this();
        extension->onAttached();
    }
    else
    {
        META_LOG_ERROR("Extension " << extension->getName() <<" already extends the object.");
    }
    return it.second;
}

bool ObjectDescriptor::removeInvokable(ObjectExtension& extension)
{
    abortIfFail(extension.getOwner().get() == p_ptr);

    auto it = extensions.find(extension.getName());
    if (it != extensions.end())
    {
        extension.onDetached();
        extension.m_descriptor->owner.reset();
        extensions.erase(it);
        return true;
    }
    META_LOG_ERROR("Extension " << extension.getName() <<" does not extend the object.");
    return false;
}


Object::Object(std::string_view name) :
    Object(name, pimpl::make_d_ptr<ObjectDescriptor>(*this))
{
}

Object::Object(std::string_view name, pimpl::d_ptr_type<ObjectDescriptor> d) :
    d_ptr(std::move(d)),
    m_name(name)
{
}

Object::~Object()
{
}

ObjectPtr Object::create(std::string_view name)
{
    return ObjectPtr(new Object(name));
}

bool Object::addExtension(ObjectExtensionPtr invokable)
{
    if (d_ptr->sealed)
    {
        META_LOG_ERROR("The object " << getName() <<" is sealed.");
        return false;
    }
    return d_ptr->addExtention(std::move(invokable));
}

bool Object::removeExtension(ObjectExtension& invokable)
{
    if (d_ptr->sealed)
    {
        META_LOG_ERROR("The object " << getName() <<" is sealed.");
        return false;
    }
    return d_ptr->removeInvokable(invokable);
}

ObjectExtensionPtr Object::findExtension(std::string_view name) const
{
    auto it = d_ptr->extensions.find(name);
    return (it != d_ptr->extensions.end()) ? it->second : ObjectExtensionPtr();
}

std::optional<ArgumentData> invoke(ObjectPtr object, std::string_view name, const PackagedArguments& arguments)
{
    abortIfFail(object && !name.empty());

    auto extension = object->findExtension(name);
    if (!extension)
    {
        return std::nullopt;
    }
    return extension->execute(arguments);
}

}
