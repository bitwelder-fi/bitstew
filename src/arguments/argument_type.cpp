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

#include <meta/arguments/argument_type.hpp>

namespace meta
{

ArgumentData PackagedArguments::get(size_t index) const
{
    return m_pack.at(index);
}

size_t PackagedArguments::getSize() const
{
    return m_pack.size();
}

bool PackagedArguments::isEmpty() const
{
    return m_pack.empty();
}

}
