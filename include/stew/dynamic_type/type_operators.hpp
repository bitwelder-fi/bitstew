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

#ifndef STEW_TYPE_OPERATORS_HPP
#define STEW_TYPE_OPERATORS_HPP

#include <stew/stew_api.hpp>
#include <stew/dynamic_type/type_info.hpp>

#include <any>

namespace stew
{

class STEW_API TypeOperators
{
public:
    struct STEW_API VTable
    {
        std::any (*add)(const std::any&, const std::any&) = nullptr;
        std::any (*sub)(const std::any&, const std::any&) = nullptr;
        std::any (*mul)(const std::any&, const std::any&) = nullptr;
        std::any (*div)(const std::any&, const std::any&) = nullptr;
    };

    TypeOperators() = default;

    TypeOperators(VTable vTable);

    ~TypeOperators() = default;

    bool isValid() const;

    operator bool() const
    {
        return isValid();
    }

    std::any add(const std::any& lhs, const std::any& rhs);
    std::any sub(const std::any& lhs, const std::any& rhs);
    std::any mul(const std::any& lhs, const std::any& rhs);
    std::any div(const std::any& lhs, const std::any& rhs);

private:
    VTable v_table;
};

}

#endif // STEW_CONVERTER_HPP
