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

#ifndef META_DETAIL_PACKAGED_ARGUMENTS_HPP
#define META_DETAIL_PACKAGED_ARGUMENTS_HPP

#include <tuple>
#include <utils/function_traits.hpp>

namespace meta {

class ObjectExtension;

namespace detail {

template <typename Function, class ArgPack>
struct PackToTuple
{
    template <int Index, int PackIndex>
    static auto convert(const ArgPack& arguments)
    {
        if constexpr (Index == 0)
        {
            return std::tuple<>();
        }
        else
        {
            using ArgType = typename traits::function_traits<Function>::signature::template get<Index - 1>::type;
            return std::tuple_cat(convert<Index - 1, PackIndex - 1>(arguments), std::make_tuple(arguments.template get<ArgType>(PackIndex - 1)));
        }
    }
};


template <typename Function>
struct enableRepack
{
    static constexpr bool packObject = std::is_member_function_pointer_v<Function>;
    static constexpr bool packSelf = traits::is_base_arg_of<ObjectExtension*, Function, 0u>::value;
    static constexpr bool value = packObject || packSelf;
};

}} // meta::detail

#endif
