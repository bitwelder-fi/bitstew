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

#ifndef STEW_ARGUMENT_HPP
#define STEW_ARGUMENT_HPP

#include <stew/stew_api.hpp>

#include <any>
#include <memory>
#include <optional>
#include <string>

namespace stew
{

/// The exception thrown when the argument type to retrieve differs from the stored type.
class STEW_API BadArgumentException : public std::exception
{
public:
    BadArgumentException(const std::type_info& actualType, const std::type_info& expectedType) noexcept;
    BadArgumentException(const BadArgumentException&) noexcept;
    ~BadArgumentException() noexcept = default;
    const char* what() const noexcept override;

private:
    std::unique_ptr<char, void(*)(void*)> message;
};

/// Stores the type and the value of an argument passed on object extension invocation.
class STEW_API Argument : public std::any
{
public:
    /// Contains the type info of an argument.
    struct STEW_API Type
    {
        Type(const std::type_info& tinfo) :
            type(tinfo)
        {
        }
        /// Returns the name of the type.
        std::string getName() const;

        /// Returns the type info of the type.
        const std::type_info& getType() const
        {
            return type;
        }

    private:
        const std::type_info& type;
    };
    /// Creates a default argument data, with no data stored.
    Argument() = default;

    /// Creates an argument data with a value.
    /// \tparam T The type of the value passed as argument.
    /// \param value The value to store.
    template <class T>
    Argument(T value) :
        std::any(value),
        m_isConst(std::is_const_v<T>)
    {
    }

    /// Returns the type of the argument.
    Type getType() const;

    /// Cast operator, returns the data stored by an Argument instance.
    /// \tparam T The type of the casted value.
    /// \return The value stored.
    /// \throws Throws std::bad_any_cast if the type to cast to is not the type the data is stored.
    template <class T>
    operator T() const;

private:
    bool m_isConst = false;
};

using ReturnValue = std::optional<Argument>;

// ----- Implementation -----
template <typename T>
Argument::operator T() const
{
    try
    {
        return m_isConst ? std::any_cast<const T>(*this) : std::any_cast<T>(*this);
    }
    catch (std::bad_any_cast&)
    {
        throw BadArgumentException(type(), typeid(T));
    }
}

}

#endif
