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

#ifndef META_OBJECT_P_HPP
#define META_OBJECT_P_HPP

#include <meta/object.hpp>

#include <unordered_map>

namespace meta
{

class ObjectDescriptor
{
public:
    DECLARE_PUBLIC(Object, ObjectDescriptor)

    Object* p_ptr = nullptr;

    explicit ObjectDescriptor(Object& owner);

    bool addExtention(ObjectExtensionPtr extension);
    bool removeInvokable(ObjectExtension& extension);

    using ExtensionsMap = std::unordered_map<std::string_view, ObjectExtensionPtr>;

    ExtensionsMap extensions;
    std::atomic_bool sealed = false;
};

}

#endif
