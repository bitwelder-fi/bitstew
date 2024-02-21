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

#include <meta/arguments/argument.hpp>
#include <meta/arguments/packaged_arguments.hpp>

namespace meta
{

PackagedArguments::Descriptor::Descriptor(Iterator begin, Iterator end) :
    pack(begin, end)
{
}

void PackagedArguments::deepCopyIfRequired()
{
    if (!m_descriptor.unique())
    {
        m_descriptor = std::make_shared<Descriptor>(m_descriptor->pack.begin(), m_descriptor->pack.end());
    }
}

PackagedArguments::PackagedArguments() :
    m_descriptor(std::make_shared<Descriptor>())
{
}

PackagedArguments::PackagedArguments(PackagedArguments&& other) :
    m_descriptor(std::make_shared<Descriptor>())
{
    swap(other);
}
PackagedArguments::PackagedArguments(const PackagedArguments& other) :
    m_descriptor(other.m_descriptor)
{
}

PackagedArguments::PackagedArguments(Iterator begin, Iterator end) :
    m_descriptor(std::make_shared<Descriptor>(begin, end))
{
}

PackagedArguments& PackagedArguments::operator=(PackagedArguments&& other)
{
    PackagedArguments tmp(std::forward<PackagedArguments>(other));
    swap(tmp);
    return *this;
}

PackagedArguments& PackagedArguments::operator=(const PackagedArguments& other)
{
    m_descriptor = other.m_descriptor;
    return *this;
}

void PackagedArguments::swap(PackagedArguments& other)
{
    std::swap(m_descriptor, other.m_descriptor);
}

PackagedArguments& PackagedArguments::operator+=(const PackagedArguments& rhs)
{
    return cat(rhs);
}

PackagedArguments& PackagedArguments::operator+=(Argument rhs)
{
    return addBack(rhs);
}

PackagedArguments& PackagedArguments::cat(const PackagedArguments& package)
{
    deepCopyIfRequired();
    m_descriptor->pack.insert(m_descriptor->pack.end(), package.m_descriptor->pack.begin(), package.m_descriptor->pack.end());
    return *this;
}

PackagedArguments& PackagedArguments::prepend(const PackagedArguments& package)
{
    deepCopyIfRequired();
    m_descriptor->pack.insert(m_descriptor->pack.begin(), package.m_descriptor->pack.begin(), package.m_descriptor->pack.end());
    return *this;
}

PackagedArguments& PackagedArguments::addBack(Argument argument)
{
    deepCopyIfRequired();
    m_descriptor->pack.push_back(argument);
    return *this;
}

PackagedArguments& PackagedArguments::addFront(Argument argument)
{
    deepCopyIfRequired();
    m_descriptor->pack.insert(m_descriptor->pack.begin(), argument);
    return *this;
}

Argument PackagedArguments::get(std::size_t index) const
{
    return m_descriptor->pack.at(index);
}

std::size_t PackagedArguments::getSize() const
{
    return m_descriptor->pack.size();
}

bool PackagedArguments::isEmpty() const
{
    return m_descriptor->pack.empty();
}

bool operator==(const PackagedArguments& lhs, const PackagedArguments& rhs)
{
    return lhs.m_descriptor == rhs.m_descriptor;
}

bool operator!=(const PackagedArguments& lhs, const PackagedArguments& rhs)
{
    return lhs.m_descriptor != rhs.m_descriptor;
}


}
