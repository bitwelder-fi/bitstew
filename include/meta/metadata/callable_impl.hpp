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

template<class Function>
Callable::Callable(std::string_view name, Function function)
{
    auto invokable = [function](const PackagedArguments& arguments) -> std::optional<ArgumentData>
    {
        if constexpr (std::is_void_v<typename traits::function_traits<Function>::return_type>)
        {
            meta::invoke(function, arguments);
            return {};
        }
        else
        {
            auto ret = meta::invoke(function, arguments);
            return ArgumentData(ret);
        }
    };
    m_descriptor.invokable = std::move(invokable);
    m_descriptor.name = name;
}

}
