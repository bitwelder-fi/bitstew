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

#include <meta/metadata/invokable.hpp>

namespace meta
{

Invokable::Invokable(Invokable&& other)
{
    swap(other);
}

Invokable& Invokable::operator=(Invokable&& other)
{
    Invokable tmp(std::forward<Invokable>(other));
    swap(tmp);
    return *this;
}

void Invokable::swap(Invokable& other)
{
    std::swap(other.m_descriptor.name, m_descriptor.name);
    std::swap(other.m_descriptor.invokable, m_descriptor.invokable);
}

bool Invokable::isValid() const
{
    return static_cast<bool>(m_descriptor.invokable);
}

}
