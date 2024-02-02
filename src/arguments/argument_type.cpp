/*
 * Copyright (C) 2023 bitWelder
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

#include <meta/arguments/argument_type.hpp>

#include <cstdlib>
#include <cstring>
#include <cxxabi.h>

namespace meta
{

BadArgument::BadArgument(const std::type_info& actualType, const std::type_info& expectedType) noexcept :
    message(nullptr, std::free)
{
    auto actual = ArgumentData::Type(actualType).getName();
    auto expected = ArgumentData::Type(expectedType).getName();
    auto msg = "Bad argument: actual type: " + actual + ", expected: " + expected;
    message.reset(static_cast<char*>(std::calloc(msg.length(), sizeof(char))));
    memcpy(message.get(), msg.c_str(), msg.length());
}

BadArgument::BadArgument(const BadArgument& other) noexcept :
    message(nullptr, std::free)
{
    const auto* msg = other.what();
    const auto msgLen = strlen(msg);
    message.reset(static_cast<char*>(std::calloc(msgLen, sizeof(char))));
    memcpy(message.get(), msg, msgLen);
}

const char* BadArgument::what() const noexcept
{
    if (!message)
    {
        return "Bad argument data.";
    }
    return message.get();
}

std::string ArgumentData::Type::getName() const
{
    int status = std::numeric_limits<int>::infinity();

    std::unique_ptr<char, void(*)(void*)> res
        {
            abi::__cxa_demangle(type.name(), NULL, NULL, &status),
            std::free
        };

    return (status == 0) ? res.get() : type.name();
}

ArgumentData::Type ArgumentData::getType() const
{
    return Type(type());
}


PackagedArguments::PackagedArguments(PackagedArguments&& other)
{
    swap(other);
}

PackagedArguments& PackagedArguments::operator=(PackagedArguments&& other)
{
    PackagedArguments tmp(std::forward<PackagedArguments>(other));
    swap(tmp);
    return *this;
}

void PackagedArguments::swap(PackagedArguments& other)
{
    m_pack.swap(other.m_pack);
}

PackagedArguments::PackagedArguments(Iterator begin, Iterator end) :
    m_pack(begin, end)
{
}

PackagedArguments& PackagedArguments::operator+=(const PackagedArguments& rhs)
{
    return append(rhs);
}

PackagedArguments& PackagedArguments::operator+=(ArgumentData rhs)
{
    m_pack.push_back(std::move(rhs));
    return *this;
}

PackagedArguments& PackagedArguments::append(const PackagedArguments& package)
{
    m_pack.insert(m_pack.end(), package.m_pack.begin(), package.m_pack.end());
    return *this;
}

ArgumentData PackagedArguments::get(std::size_t index) const
{
    return m_pack.at(index);
}

std::size_t PackagedArguments::getSize() const
{
    return m_pack.size();
}

bool PackagedArguments::isEmpty() const
{
    return m_pack.empty();
}

}
