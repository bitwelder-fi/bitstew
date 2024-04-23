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

#ifndef STEW_BAD_VARIABLE_EXCEPTION_HPP
#define STEW_BAD_VARIABLE_EXCEPTION_HPP

#include <stew/stew_api.hpp>

#include <memory>
#include <string>

#include <exception>

namespace stew
{

struct TypeInfo;

class STEW_API Exception : public std::exception
{
public:
    Exception(const Exception&) noexcept;
    ~Exception() noexcept = default;
    const char* what() const noexcept final;

protected:
    explicit Exception() noexcept;
    void setMessage(std::string text);
    std::unique_ptr<char, void(*)(void*)> message;
};


/// The exception thrown when the argument type to retrieve differs from the stored type.
class STEW_API BadVariableException : public Exception
{
public:
    BadVariableException(const std::type_info& actualType, const std::type_info& expectedType) noexcept;
};

/// The exception thrown when type conversion is not possible.
class STEW_API ConversionException : public Exception
{
public:
    ConversionException(const TypeInfo& source, const TypeInfo& target) noexcept;
};

/// The exception thrown when the type converter reaches an unhandled state.
class STEW_API BadConverterException : public Exception
{
public:
    BadConverterException(const TypeInfo& source, const TypeInfo& target) noexcept;
};

}

#endif // STEW_BAD_VARIABLE_EXCEPTION_HPP
