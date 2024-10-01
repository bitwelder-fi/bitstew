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

std::any TypeOperators::add(const std::any& lhs, const std::any& rhs)
{
    if (!v_table.add)
    {
        throw UndefinedOperator("+");
    }
    return v_table.add(lhs, rhs);
}

std::any TypeOperators::sub(const std::any& lhs, const std::any& rhs)
{
    if (!v_table.sub)
    {
        throw UndefinedOperator("-");
    }
    return v_table.sub(lhs, rhs);
}

std::any TypeOperators::mul(const std::any& lhs, const std::any& rhs)
{
    if (!v_table.mul)
    {
        throw UndefinedOperator("*");
    }
    return v_table.mul(lhs, rhs);
}

std::any TypeOperators::div(const std::any& lhs, const std::any& rhs)
{
    if (!v_table.div)
    {
        throw UndefinedOperator("/");
    }
    return v_table.div(lhs, rhs);
}

bool TypeOperators::land(const std::any& lhs, const std::any& rhs)
{
    if (!v_table.land)
    {
        throw UndefinedOperator("&&");
    }
    return v_table.land(lhs, rhs);
}

bool TypeOperators::lor(const std::any& lhs, const std::any& rhs)
{
    if (!v_table.lor)
    {
        throw UndefinedOperator("||");
    }
    return v_table.lor(lhs, rhs);
}

std::any TypeOperators::lnot(const std::any& rhs)
{
    if (!v_table.lnot)
    {
        throw UndefinedOperator("!");
    }
    return v_table.lnot(rhs);
}

bool TypeOperators::eq(const std::any& lhs, const std::any& rhs)
{
    if (!v_table.eq)
    {
        throw UndefinedOperator("==");
    }
    return v_table.eq(lhs, rhs);
}

bool TypeOperators::less(const std::any& lhs, const std::any& rhs)
{
    if (!v_table.less)
    {
        throw UndefinedOperator("<");
    }
    return v_table.less(lhs, rhs);
}

bool TypeOperators::leq(const std::any& lhs, const std::any& rhs)
{
    if (!v_table.leq)
    {
        throw UndefinedOperator("<=");
    }
    return v_table.leq(lhs, rhs);
}

bool TypeOperators::gt(const std::any& lhs, const std::any& rhs)
{
    if (!v_table.gt)
    {
        throw UndefinedOperator(">");
    }
    return v_table.gt(lhs, rhs);
}

bool TypeOperators::geq(const std::any& lhs, const std::any& rhs)
{
    if (!v_table.geq)
    {
        throw UndefinedOperator(">=");
    }
    return v_table.geq(lhs, rhs);
}

std::any TypeOperators::bw_and(const std::any& lhs, const std::any& rhs)
{
    if (!v_table.bw_and)
    {
        throw UndefinedOperator("&");
    }
    return v_table.bw_and(lhs, rhs);
}

std::any TypeOperators::bw_or(const std::any& lhs, const std::any& rhs)
{
    if (!v_table.bw_or)
    {
        throw UndefinedOperator("|");
    }
    return v_table.bw_or(lhs, rhs);
}

std::any TypeOperators::bw_xor(const std::any& lhs, const std::any& rhs)
{
    if (!v_table.bw_xor)
    {
        throw UndefinedOperator("~");
    }
    return v_table.bw_xor(lhs, rhs);
}

std::any TypeOperators::bw_not(const std::any& lhs)
{
    if (!v_table.bw_not)
    {
        throw UndefinedOperator("!");
    }
    return v_table.bw_not(lhs);
}

std::any TypeOperators::bw_shl(const std::any& value, std::size_t count)
{
    if (!v_table.bw_shl)
    {
        throw UndefinedOperator("<<");
    }
    return v_table.bw_shl(value, count);
}

std::any TypeOperators::bw_shr(const std::any& value, std::size_t count)
{
    if (!v_table.bw_shr)
    {
        throw UndefinedOperator(">>");
    }
    return v_table.bw_shr(value, count);
}

void* TypeOperators::ptr(const std::any& lhs)
{
    if (!v_table.ptr)
    {
        throw UndefinedOperator("->");
    }
    return v_table.ptr(lhs);
}

const void* TypeOperators::cptr(const std::any& lhs)
{
    if (!v_table.cptr)
    {
        throw UndefinedOperator("(const)->");
    }
    return v_table.cptr(lhs);
}


}
