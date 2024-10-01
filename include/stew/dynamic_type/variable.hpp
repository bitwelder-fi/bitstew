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

#ifndef STEW_VARIABLE_HPP
#define STEW_VARIABLE_HPP

#include <stew/dynamic_type/exceptions.hpp>
#include <stew/dynamic_type/type_converter.hpp>
#include <stew/dynamic_type/type_info.hpp>
#include <stew/dynamic_type/type_registry.hpp>

#include <any>

namespace stew
{

class Variable;

namespace
{
template <typename T>
struct is_any : std::false_type {};

template <>
struct is_any<std::any> : std::true_type {};

template <typename T>
concept generic_type = !is_any<T>::value && !std::is_same_v<Variable, T>;
}

/// A dynamic type. Stores only copy-constructible type values.
///
class STEW_API Variable
{
    std::any m_data;
    TypeOperators* m_ops = nullptr;

public:
    /// Default constructor.
    Variable() = default;

    /// Constructor. Creates a variable from an any.
    Variable(std::any data);
    /// Constructor. Creates a variable from a value that is not an any or a Variable.
    template <typename T>
        requires generic_type<T>
    Variable(T value) :
        Variable(std::any(value))
    {
    }

    /// Assignment operator.
    Variable& operator=(const std::any& data);
    /// Assignment operator.
    Variable& operator=(std::any&& data);
    /// Assignment operator.
    template <typename T>
        requires generic_type<T>
    Variable& operator=(T&& data)
    {
        m_data = std::forward<T>(data);
        return *this;
    }

    /// Returns the current type of the variable.
    TypeInfo type() const;

    /// Returns if this variable is of RTTI type.
    bool isTypeOf(const std::type_info& type) const;

    /// Returns if this variable is of type T.
    template <typename T>
    bool isTypeOf()
    {
        return isTypeOf(typeid(T));
    }    

    /// Cast operator, returns the data stored by the Variable.
    /// \tparam T The type of the casted value.
    /// \return The value stored.
    /// \throws Throws std::bad_any_cast if the type to cast to is not the type the data is stored.
    template <class T>
    operator T() const;

    /// \name Operators
    /// \{

    Variable& operator +=(const Variable& rhs);
    Variable& operator -=(const Variable& rhs);
    Variable& operator *=(const Variable& rhs);
    Variable& operator /=(const Variable& rhs);

    Variable& operator&=(const Variable& rhs);
    Variable& operator|=(const Variable& rhs);
    Variable& operator^=(const Variable& rhs);
    Variable& operator <<=(std::size_t count);
    Variable& operator >>=(std::size_t count);

    void* operator ->();
    const void* operator ->() const;

    friend bool operator &&(const Variable& lhs, const Variable& rhs);
    friend bool operator ||(const Variable& lhs, const Variable& rhs);
    friend bool operator ==(const Variable& lhs, const Variable& rhs);
    friend bool operator <(const Variable& lhs, const Variable& rhs);
    friend bool operator <=(const Variable& lhs, const Variable& rhs);
    friend bool operator >(const Variable& lhs, const Variable& rhs);
    friend bool operator >=(const Variable& lhs, const Variable& rhs);
    friend Variable operator &(const Variable& lhs, const Variable& rhs);
    friend Variable operator |(const Variable& lhs, const Variable& rhs);
    friend Variable operator ^(const Variable& lhs, const Variable& rhs);
    friend Variable operator !(const Variable& rhs);
    friend Variable operator ~(const Variable& rhs);
    friend Variable operator <<(const Variable& lhs, std::size_t count);
    friend Variable operator >>(const Variable& lhs, std::size_t count);

    /// \}

private:
    friend std::any convert(Variable& value, const TypeInfo& targetType);
    friend std::any convert(const Variable& value, const TypeInfo& targetType);
};

/// \name Free functions
/// \{

/// Converts a variable to a target type. Returns the type-safe container with the converted value.
/// \param value The value to convert.
/// \param targetType The type to convert the value.
/// \return The converted value
/// \throws ConversionException - when cannot convert the value to target type.
/// \throws InvalidConverter - when the type converter is invalid.
STEW_API std::any convert(Variable& value, const TypeInfo& targetType);
STEW_API std::any convert(const Variable& value, const TypeInfo& targetType);

STEW_API Variable operator +(const Variable& lhs, const Variable& rhs);
STEW_API Variable operator -(const Variable& lhs, const Variable& rhs);
STEW_API Variable operator *(const Variable& lhs, const Variable& rhs);
STEW_API Variable operator /(const Variable& lhs, const Variable& rhs);
STEW_API bool operator &&(const Variable& lhs, const Variable& rhs);
STEW_API bool operator ||(const Variable& lhs, const Variable& rhs);
STEW_API bool operator ==(const Variable& lhs, const Variable& rhs);
STEW_API bool operator <(const Variable& lhs, const Variable& rhs);
STEW_API bool operator <=(const Variable& lhs, const Variable& rhs);
STEW_API bool operator >(const Variable& lhs, const Variable& rhs);
STEW_API bool operator >=(const Variable& lhs, const Variable& rhs);
STEW_API Variable operator &(const Variable& lhs, const Variable& rhs);
STEW_API Variable operator |(const Variable& lhs, const Variable& rhs);
STEW_API Variable operator ^(const Variable& lhs, const Variable& rhs);
STEW_API Variable operator !(const Variable& rhs);
STEW_API Variable operator ~(const Variable& rhs);
STEW_API Variable operator <<(const Variable& lhs, std::size_t count);
STEW_API Variable operator >>(const Variable& lhs, std::size_t count);

template <class T>
STEW_TEMPLATE_API Variable operator +(const Variable& lhs, const T& rhs)
{
    Variable result = lhs;
    result += rhs;
    return result;
}
template <class T>
STEW_TEMPLATE_API Variable operator +(const T& lhs, const Variable& rhs)
{
    Variable result = lhs;
    result += rhs;
    return result;
}

template <class T>
STEW_TEMPLATE_API Variable operator -(const Variable& lhs, const T& rhs)
{
    Variable result = lhs;
    result -= rhs;
    return result;
}
template <class T>
STEW_TEMPLATE_API Variable operator -(const T& lhs, const Variable& rhs)
{
    Variable result = lhs;
    result -= rhs;
    return result;
}

template <class T>
STEW_TEMPLATE_API Variable operator *(const Variable& lhs, const T& rhs)
{
    Variable result = lhs;
    result *= rhs;
    return result;
}
template <class T>
STEW_TEMPLATE_API Variable operator *(const T& lhs, const Variable& rhs)
{
    Variable result = lhs;
    result *= rhs;
    return result;
}

template <class T>
STEW_TEMPLATE_API Variable operator /(const Variable& lhs, const T& rhs)
{
    Variable result = lhs;
    result /= rhs;
    return result;
}
template <class T>
STEW_TEMPLATE_API Variable operator /(const T& lhs, const Variable& rhs)
{
    Variable result = lhs;
    result /= rhs;
    return result;
}

/// \}

//------------------Implementation------------------
template <class T>
Variable::operator T() const
{
    if (typeid(T) == m_data.type())
    {
        return std::any_cast<T>(m_data);
    }

    return std::any_cast<T>(convert(*this, typeid(T)));
}

}

#endif // STEW_VARIABLE_HPP
