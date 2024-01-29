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

ObjectExtension::ObjectExtension(std::string_view name, pimpl::d_ptr_type<Descriptor> descriptor) :
    MetaObject(name),
    m_descriptor(std::move(descriptor))
{
}

ObjectPtr ObjectExtension::getOwner() const
{
    abortIfFail(m_descriptor);
    return m_descriptor->owner.lock();
}

ArgumentData ObjectExtension::execute(const PackagedArguments& arguments)
{
    abortIfFail(m_descriptor);

    return executeOverride(arguments);
}

}
