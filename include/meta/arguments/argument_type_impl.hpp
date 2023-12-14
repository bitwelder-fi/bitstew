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

#include <meta/log/trace.hpp>
#include <array>
#include <tuple>

namespace meta
{

namespace detail
{

template <typename Function>
struct PackToTuple
{
    template <int Index, int PackIndex>
    static auto convert(const PackagedArguments& arguments)
    {
        if constexpr (Index == 0)
        {
            return std::tuple<>();
        }
        else
        {
            using ArgType = typename traits::function_traits<Function>::template argument<Index - 1>::type;
            return std::tuple_cat(convert<Index - 1, PackIndex - 1>(arguments), std::make_tuple(arguments.get<ArgType>(PackIndex - 1)));
        }
    }
};

}

template <typename T>
ArgumentData::operator T() const
{
    try
    {
        return std::any_cast<T>(*this);
    }
    catch (std::bad_any_cast& e)
    {
        META_LOG_ERROR("Bad cast of argument data. Expected " << this->type().name());
        return {};
    }
}

template <typename... Arguments>
PackagedArguments::PackagedArguments(Arguments&&... arguments)
{
    std::array<ArgumentData, sizeof... (Arguments)> aa = {{ArgumentData(arguments)...}};
    m_pack.reserve(aa.size());
    m_pack.insert(m_pack.end(), aa.begin(), aa.end());
}

template <typename T>
T PackagedArguments::get(std::size_t index) const
{
    return m_pack.at(index);
}


template <class FunctionSignature>
auto PackagedArguments::toTuple() const
{
    constexpr std::size_t N = traits::function_traits<FunctionSignature>::arity;

    if constexpr (std::is_member_function_pointer_v<FunctionSignature>)
    {
        using ClassType = typename traits::function_traits<FunctionSignature>::object;
        auto object = static_cast<ClassType*>(m_pack.front());
        return std::tuple_cat(std::make_tuple(static_cast<ClassType*>(object)),
                              detail::PackToTuple<FunctionSignature>::template convert<N, N + 1>(*this));
    }
    else
    {
        return detail::PackToTuple<FunctionSignature>::template convert<N, N>(*this);
    }
}

}
