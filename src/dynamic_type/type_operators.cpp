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

#include <stew/dynamic_type/exceptions.hpp>
#include <stew/dynamic_type/type_operators.hpp>

namespace stew
{
TypeOperators::TypeOperators(VTable vTable) :
    v_table(std::move(vTable))
{
}

bool TypeOperators::isValid() const
{
    return v_table.add || v_table.sub || v_table.mul || v_table.div;
}

std::any TypeOperators::add(const std::any& lhs, const std::any& rhs)
{
    if (!isValid())
    {
        throw UndefinedOperator("+");
    }
    return v_table.add(lhs, rhs);
}

std::any TypeOperators::sub(const std::any& lhs, const std::any& rhs)
{
    if (!isValid())
    {
        throw UndefinedOperator("-");
    }
    return v_table.sub(lhs, rhs);
}

std::any TypeOperators::mul(const std::any& lhs, const std::any& rhs)
{
    if (!isValid())
    {
        throw UndefinedOperator("*");
    }
    return v_table.mul(lhs, rhs);
}

std::any TypeOperators::div(const std::any& lhs, const std::any& rhs)
{
    if (!isValid())
    {
        throw UndefinedOperator("/");
    }
    return v_table.div(lhs, rhs);
}

}
