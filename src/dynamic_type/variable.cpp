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

#include <stew/dynamic_type/variable.hpp>

namespace stew
{

TypeInfo Variable::type() const
{
    if (!m_data.has_value())
    {
        throw std::bad_typeid();
    }
    return TypeInfo(m_data.type());
}

bool Variable::isTypeOf(const std::type_info& type) const
{
    return m_data.type() == type;
}

Variable& Variable::operator +=(const Variable& rhs)
{
    if (auto& ops = TypeRegistry::instance().getTypeOperators(type()); type() != rhs.type())
    {
        auto rvalue = convert(rhs, type());
        m_data = ops.add(m_data, rvalue);
    }
    else
    {
        m_data = ops.add(m_data, rhs.m_data);
    }

    return *this;
}

Variable& Variable::operator -=(const Variable& rhs)
{
    if (auto& ops = TypeRegistry::instance().getTypeOperators(type()); type() != rhs.type())
    {
        auto rvalue = convert(rhs, type());
        m_data = ops.sub(m_data, rvalue);
    }
    else
    {
        m_data = ops.sub(m_data, rhs.m_data);
    }

    return *this;
}

Variable& Variable::operator *=(const Variable& rhs)
{
    if (auto& ops = TypeRegistry::instance().getTypeOperators(type()); type() != rhs.type())
    {
        auto rvalue = convert(rhs, type());
        m_data = ops.mul(m_data, rvalue);
    }
    else
    {
        m_data = ops.mul(m_data, rhs.m_data);
    }

    return *this;
}

Variable& Variable::operator /=(const Variable& rhs)
{
    if (auto& ops = TypeRegistry::instance().getTypeOperators(type()); type() != rhs.type())
    {
        auto rvalue = convert(rhs, type());
        m_data = ops.div(m_data, rvalue);
    }
    else
    {
        m_data = ops.div(m_data, rhs.m_data);
    }

    return *this;
}


std::any convert(Variable& value, const TypeInfo& targetType)
{
    if (auto converter = TypeRegistry::instance().findConverter(value.type(), targetType); converter)
    {
        return converter->convert(value.m_data);
    }

    throw ConversionException(value.type(), targetType);
}

std::any convert(const Variable& value, const TypeInfo& targetType)
{
    if (auto converter = TypeRegistry::instance().findConverter(value.type(), targetType); converter)
    {
        return converter->convert(value.m_data);
    }

    throw ConversionException(value.type(), targetType);
}

}
