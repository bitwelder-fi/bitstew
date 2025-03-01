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

Variable::Variable(std::any data) :
    m_data(data),
    m_ops(&TypeRegistry::instance().getTypeOperators(type()))
{
}

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

Variable& Variable::operator=(const std::any& data)
{
    m_data = data;
    m_ops = &TypeRegistry::instance().getTypeOperators(type());
    return *this;
}

Variable& Variable::operator=(std::any&& data)
{
    m_data = std::forward<std::any>(data);
    m_ops = &TypeRegistry::instance().getTypeOperators(type());
    return *this;
}


Variable& Variable::operator +=(const Variable& rhs)
{
    if (!m_ops)
    {
        *this = rhs.m_data;
    }
    else if (type() != rhs.type())
    {
        auto rvalue = convert(rhs, type());
        m_data = m_ops->add(m_data, rvalue);
    }
    else
    {
        m_data = m_ops->add(m_data, rhs.m_data);
    }

    return *this;
}

Variable& Variable::operator -=(const Variable& rhs)
{
    if (!m_ops)
    {
        *this = rhs.m_data;
        *this *= -1;
    }
    else if (type() != rhs.type())
    {
        auto rvalue = convert(rhs, type());
        m_data = m_ops->sub(m_data, rvalue);
    }
    else
    {
        m_data = m_ops->sub(m_data, rhs.m_data);
    }

    return *this;
}

Variable& Variable::operator *=(const Variable& rhs)
{
    if (m_ops)
    {
        if (type() != rhs.type())
        {
            auto rvalue = convert(rhs, type());
            m_data = m_ops->mul(m_data, rvalue);
        }
        else
        {
            m_data = m_ops->mul(m_data, rhs.m_data);
        }
    }

    return *this;
}

Variable& Variable::operator /=(const Variable& rhs)
{
    if (m_ops)
    {
        if (type() != rhs.type())
        {
            auto rvalue = convert(rhs, type());
            m_data = m_ops->div(m_data, rvalue);
        }
        else
        {
            m_data = m_ops->div(m_data, rhs.m_data);
        }
    }

    return *this;
}

Variable& Variable::operator&=(const Variable& rhs)
{
    if (m_ops)
    {
        if (type() != rhs.type())
        {
            auto rvalue = convert(rhs, type());
            m_data = m_ops->bw_and(m_data, rvalue);
        }
        else
        {
            m_data = m_ops->bw_and(m_data, rhs.m_data);
        }
    }

    return *this;
}

Variable& Variable::operator|=(const Variable& rhs)
{
    if (!m_ops)
    {
        *this = rhs.m_data;
    }
    else if (type() != rhs.type())
    {
        auto rvalue = convert(rhs, type());
        m_data = m_ops->bw_or(m_data, rvalue);
    }
    else
    {
        m_data = m_ops->bw_or(m_data, rhs.m_data);
    }
    return *this;
}

Variable& Variable::operator^=(const Variable& rhs)
{
    if (!m_ops)
    {
        *this = rhs.m_data;
    }
    else if (type() != rhs.type())
    {
        auto rvalue = convert(rhs, type());
        m_data = m_ops->bw_xor(m_data, rvalue);
    }
    else
    {
        m_data = m_ops->bw_xor(m_data, rhs.m_data);
    }
    return *this;
}

Variable& Variable::operator <<=(std::size_t count)
{
    if (m_ops)
    {
        m_data = m_ops->bw_shl(m_data, count);
    }
    return *this;
}

Variable& Variable::operator >>=(std::size_t count)
{
    if (m_ops)
    {
        m_data = m_ops->bw_shr(m_data, count);
    }
    return *this;
}

void* Variable::operator ->()
{
    if (m_ops)
    {
        return m_ops->ptr(m_data);
    }
    return {};
}

const void* Variable::operator ->() const
{
    if (m_ops)
    {
        return m_ops->cptr(m_data);
    }
    return {};
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

Variable operator +(const Variable& lhs, const Variable& rhs)
{
    Variable result = lhs;
    result += rhs;
    return result;
}

Variable operator -(const Variable& lhs, const Variable& rhs)
{
    Variable result = lhs;
    result -= rhs;
    return result;
}

Variable operator *(const Variable& lhs, const Variable& rhs)
{
    Variable result = lhs;
    result *= rhs;
    return result;
}

Variable operator /(const Variable& lhs, const Variable& rhs)
{
    Variable result = lhs;
    result /= rhs;
    return result;

}

bool operator &&(const Variable& lhs, const Variable& rhs)
{
    if (lhs.type() != rhs.type())
    {
        auto rvalue = convert(rhs, lhs.type());
        return lhs.m_ops->land(lhs.m_data, rvalue);
    }
    else
    {
        return lhs.m_ops->land(lhs.m_data, rhs.m_data);
    }
}

bool operator ||(const Variable& lhs, const Variable& rhs)
{
    if (lhs.type() != rhs.type())
    {
        auto rvalue = convert(rhs, lhs.type());
        return lhs.m_ops->lor(lhs.m_data, rvalue);
    }
    else
    {
        return lhs.m_ops->lor(lhs.m_data, rhs.m_data);
    }
}

bool operator ==(const Variable& lhs, const Variable& rhs)
{
    if (lhs.type() != rhs.type())
    {
        auto rvalue = convert(rhs, lhs.type());
        return lhs.m_ops->eq(lhs.m_data, rvalue);
    }
    else
    {
        return lhs.m_ops->eq(lhs.m_data, rhs.m_data);
    }
}

bool operator <(const Variable& lhs, const Variable& rhs)
{
    if (lhs.type() != rhs.type())
    {
        auto rvalue = convert(rhs, lhs.type());
        return lhs.m_ops->less(lhs.m_data, rvalue);
    }
    else
    {
        return lhs.m_ops->less(lhs.m_data, rhs.m_data);
    }
}

bool operator <=(const Variable& lhs, const Variable& rhs)
{
    if (lhs.type() != rhs.type())
    {
        auto rvalue = convert(rhs, lhs.type());
        return lhs.m_ops->leq(lhs.m_data, rvalue);
    }
    else
    {
        return lhs.m_ops->leq(lhs.m_data, rhs.m_data);
    }
}

bool operator >(const Variable& lhs, const Variable& rhs)
{
    if (lhs.type() != rhs.type())
    {
        auto rvalue = convert(rhs, lhs.type());
        return lhs.m_ops->gt(lhs.m_data, rvalue);
    }
    else
    {
        return lhs.m_ops->gt(lhs.m_data, rhs.m_data);
    }
}

bool operator >=(const Variable& lhs, const Variable& rhs)
{
    if (lhs.type() != rhs.type())
    {
        auto rvalue = convert(rhs, lhs.type());
        return lhs.m_ops->geq(lhs.m_data, rvalue);
    }
    else
    {
        return lhs.m_ops->geq(lhs.m_data, rhs.m_data);
    }
}

Variable operator &(const Variable& lhs, const Variable& rhs)
{
    if (lhs.type() != rhs.type())
    {
        auto rvalue = convert(rhs, lhs.type());
        return lhs.m_ops->bw_and(lhs.m_data, rvalue);
    }
    else
    {
        return lhs.m_ops->bw_and(lhs.m_data, rhs.m_data);
    }

}

Variable operator |(const Variable& lhs, const Variable& rhs)
{
    if (lhs.type() != rhs.type())
    {
        auto rvalue = convert(rhs, lhs.type());
        return lhs.m_ops->bw_or(lhs.m_data, rvalue);
    }
    else
    {
        return lhs.m_ops->bw_or(lhs.m_data, rhs.m_data);
    }
}

Variable operator ^(const Variable& lhs, const Variable& rhs)
{
    if (lhs.type() != rhs.type())
    {
        auto rvalue = convert(rhs, lhs.type());
        return lhs.m_ops->bw_xor(lhs.m_data, rvalue);
    }
    else
    {
        return lhs.m_ops->bw_xor(lhs.m_data, rhs.m_data);
    }
}

Variable operator !(const Variable& rhs)
{
    if (rhs.m_ops)
    {
        return rhs.m_ops->lnot(rhs.m_data);
    }

    return rhs;
}

Variable operator ~(const Variable& rhs)
{
    if (rhs.m_ops)
    {
        return rhs.m_ops->bw_not(rhs.m_data);
    }

    return rhs;
}

Variable operator <<(const Variable& lhs, std::size_t count)
{
    if (lhs.m_ops)
    {
        return lhs.m_ops->bw_shl(lhs.m_data, count);
    }

    return lhs;
}

Variable operator >>(const Variable& lhs, std::size_t count)
{
    if (lhs.m_ops)
    {
        return lhs.m_ops->bw_shr(lhs.m_data, count);
    }

    return lhs;
}

}
