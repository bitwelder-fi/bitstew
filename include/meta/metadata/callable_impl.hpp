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

namespace meta
{

namespace detail
{

template <typename Function>
auto invokeFunction(Function function, const PackagedArguments& arguments)
{
    if constexpr (std::is_member_function_pointer_v<Function>)
    {
        using ClassType = typename traits::function_traits<Function>::object;
        auto object = static_cast<ClassType*>(arguments.get(0u));
        const auto args = PackagedArguments(arguments.begin() + 1, arguments.end());

        auto pack = args.toTuple<Function>(object);
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

template<class Function>
Callable::Callable(std::string_view name, Function function)
{
    auto invokable = [function](const PackagedArguments& arguments) -> ArgumentData
    {
        if constexpr (std::is_void_v<typename traits::function_traits<Function>::return_type>)
        {
            detail::invokeFunction(function, arguments);
            return ArgumentData();
        }
        else
        {
            auto ret = detail::invokeFunction(function, arguments);
            return ArgumentData(ret);
        }
    };
    m_descriptor.invokable = std::move(invokable);
    m_descriptor.name = name;
}

}
