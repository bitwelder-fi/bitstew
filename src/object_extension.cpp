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

#include <meta/object_extension.hpp>
#include <meta/meta.hpp>
#include <meta/object.hpp>

namespace meta
{

ObjectExtension::ObjectExtension(std::string_view name) :
    MetaObject(name)
{
}

ObjectPtr ObjectExtension::getOwner() const
{
    return m_owner.lock();
}

ArgumentData ObjectExtension::run(const PackagedArguments& arguments)
{
    return runOverride(arguments);
}

}
