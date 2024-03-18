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

#include <meta/arguments/argument.hpp>

#include <cstdlib>
#include <cstring>
#include <cxxabi.h>
#include <limits>

namespace meta
{

BadArgumentException::BadArgumentException(const std::type_info& actualType, const std::type_info& expectedType) noexcept :
    message(nullptr, std::free)
{
    auto actual = Argument::Type(actualType).getName();
    auto expected = Argument::Type(expectedType).getName();
    auto msg = "Bad argument type:\n\tactual type: " + actual + "\n\texpected: " + expected;
    message.reset(static_cast<char*>(std::calloc(msg.length(), sizeof(char))));
    memcpy(message.get(), msg.c_str(), msg.length());
}

BadArgumentException::BadArgumentException(const BadArgumentException& other) noexcept :
    message(nullptr, std::free)
{
    const auto* msg = other.what();
    const auto msgLen = strlen(msg);
    message.reset(static_cast<char*>(std::calloc(msgLen, sizeof(char))));
    memcpy(message.get(), msg, msgLen);
}

const char* BadArgumentException::what() const noexcept
{
    if (!message)
    {
        return "Bad argument data.";
    }
    return message.get();
}

std::string Argument::Type::getName() const
{
    int status = std::numeric_limits<int>::infinity();

    std::unique_ptr<char, void(*)(void*)> res
        {
            abi::__cxa_demangle(type.name(), NULL, NULL, &status),
            std::free
        };

    return (status == 0) ? res.get() : type.name();
}

Argument::Type Argument::getType() const
{
    return Type(type());
}

}
