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
#include <tuple>

namespace meta
{

namespace detail
{

template <typename Function>
struct PackToTuple
{
    template <int Index>
    static auto convert(const PackagedArguments& arguments)
    {
        if constexpr (Index == 0)
        {
            return std::tuple<>();
        }
        else
        {
            using ArgType = typename traits::function_traits<Function>::template argument<Index - 1>::type;
            return std::tuple_cat(convert<Index - 1>(arguments), std::make_tuple(arguments.get<ArgType>(Index - 1)));
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
T PackagedArguments::get(size_t index) const
{
    return m_pack.at(index);
}


template <class FunctionSignature>
auto PackagedArguments::toTuple(FunctionSignature) const
{
    constexpr std::size_t N = traits::function_traits<FunctionSignature>::arity;
    return detail::PackToTuple<FunctionSignature>::template convert<N>(*this);
}

template <class TClass, class TRet, class... TArgs>
auto PackagedArguments::toTuple(TRet (TClass::*)(TArgs...)) const
{
    using Signature = TRet (TClass::*)(TArgs...);
    constexpr std::size_t N = traits::function_traits<Signature>::arity;
    return detail::PackToTuple<Signature>::template convert<N>(*this);
}


template <typename Function>
auto invoke(Function function, const PackagedArguments& arguments)
{
    if constexpr (std::is_member_function_pointer_v<Function>)
    {
        using ClassType = typename traits::function_traits<Function>::object;
        auto object = static_cast<ClassType*>(arguments.get(0u));
        const auto args = PackagedArguments(arguments.begin() + 1, arguments.end());

        constexpr std::size_t N = traits::function_traits<Function>::arity;
        auto pack = std::tuple_cat(std::make_tuple(object), detail::PackToTuple<Function>::template convert<N>(args));
        return std::apply(function, pack);
    }
    else
    {
        constexpr std::size_t N = traits::function_traits<Function>::arity;
        auto pack = detail::PackToTuple<Function>::template convert<N>(arguments);
        return std::apply(function, pack);
    }
}

}
