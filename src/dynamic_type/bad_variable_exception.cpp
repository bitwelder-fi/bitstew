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

#include <stew/dynamic_type/bad_variable_exception.hpp>
#include <stew/dynamic_type/type_info.hpp>

#include <cstdlib>
#include <cstring>

namespace stew
{

Exception::Exception() noexcept :
    message(nullptr, std::free)
{
}

Exception::Exception(const Exception& other) noexcept :
    message(nullptr, std::free)
{
    setMessage(other.what());
}

void Exception::setMessage(std::string text)
{
    const auto length = text.length();
    message.reset(static_cast<char*>(std::calloc(length, sizeof(char))));
    memcpy(message.get(), text.c_str(), length);
}

const char* Exception::what() const noexcept
{
    if (!message)
    {
        return "Unknown Variable Exception.";
    }
    return message.get();
}


BadVariableException::BadVariableException(const std::type_info& actualType, const std::type_info& expectedType) noexcept
{
    auto actual = TypeInfo(actualType).getName();
    auto expected = TypeInfo(expectedType).getName();
    setMessage("Bad variable type:\n\tactual type: " + actual + "\n\texpected: " + expected);
}


ConversionException::ConversionException(const TypeInfo& source, const TypeInfo& target) noexcept
{
    setMessage("Conversion error:\n\tfrom: " + source.getName() + "\n\tto: " + target.getName());
}


BadConverterException::BadConverterException(const TypeInfo& source, const TypeInfo& target) noexcept
{
    setMessage("Bad converter:\n\tfrom: " + source.getName() + "\n\tto: " + target.getName());
}

}
