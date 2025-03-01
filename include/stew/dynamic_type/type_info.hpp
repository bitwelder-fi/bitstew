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

#ifndef STEW_TYPE_INFO_HPP
#define STEW_TYPE_INFO_HPP

#include <stew/stew_api.hpp>

#include <typeindex>
#include <typeinfo>
#include <string>

namespace stew
{

/// The type info of a variable.
struct STEW_API TypeInfo
{
    TypeInfo(const std::type_info& typeInfo) :
        m_typeInfo(typeInfo)
    {
    }

    ~TypeInfo() = default;

    /// Returns the name of the type.
    std::string getName() const;

    /// Returns the type index of the type.
    std::type_index index() const;

    /// Returns the type info of the type.
    operator const std::type_info&() const
    {
        return m_typeInfo;
    }

    /// Equality comparison.
    friend bool operator==(const TypeInfo& lhs, const TypeInfo& rhs)
    {
        return lhs.m_typeInfo == rhs.m_typeInfo;
    }
    /// Equality comparison.
    friend bool operator==(const std::type_info& lhs, const TypeInfo& rhs)
    {
        return lhs == rhs.m_typeInfo;
    }

private:
    const std::type_info& m_typeInfo;
};

}

#endif // STEW_TYPE_INFO_HPP
