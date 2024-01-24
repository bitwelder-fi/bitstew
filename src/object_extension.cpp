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

ObjectExtension::Descriptor::Descriptor(std::string_view name, bool repackArguments) :
    name(name),
    repack(repackArguments)
{
    abortIfFail(isValidMetaName(name));
}


ObjectExtension::ObjectExtension(pimpl::d_ptr_type<Descriptor> descriptor) :
    m_descriptor(std::move(descriptor))
{
}

ObjectPtr ObjectExtension::getOwner() const
{
    abortIfFail(m_descriptor);
    return m_descriptor->owner.lock();
}

PackagedArguments ObjectExtension::Descriptor::repackArguments(const PackagedArguments& arguments)
{
    return PackagedArguments(arguments.begin(), arguments.end());
}

ArgumentData ObjectExtension::execute(const PackagedArguments& arguments)
{
    abortIfFail(m_descriptor);

    if (m_descriptor->repack)
    {
        const auto repack = m_descriptor->repackArguments(arguments);
        return m_descriptor->execute(repack);
    }

    return m_descriptor->execute(arguments);
}

}
