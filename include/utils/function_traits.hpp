/*
 * Copyright (C) 2017-2019 bitWelder
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

#ifndef WRAP_FUNCTION_TRAITS_HPP
#define WRAP_FUNCTION_TRAITS_HPP

#include <tuple>
#include <type_traits>

namespace traits
{

/// Traits for functors and function objects.
template <typename Function>
struct function_traits : public function_traits<decltype(&Function::operator())>
{
};

/// Method traits.
template <class TObject, typename TRet, typename... Args>
struct function_traits<TRet(TObject::*)(Args...)>
{
    using object = TObject;
    using return_type = TRet;
    using arg_types = std::tuple<Args...>;
    typedef TRet(TObject::*function_type)(Args...);

    static constexpr std::size_t arity = sizeof... (Args);
    static constexpr bool is_const = false;

    template <std::size_t N>
    struct argument
    {
        static_assert(N < arity, "error: invalid parameter index.");
        using type = typename std::tuple_element<N, std::tuple<Args...>>::type;
    };

    template <typename... TestArgs>
    static constexpr bool is_same_args = std::is_same_v<std::tuple<Args...>, std::tuple<TestArgs...>>;
};

/// Const method traits.
template <class TObject, typename TRet, typename... Args>
struct function_traits<TRet(TObject::*)(Args...) const>
{
    using object = TObject const;
    using return_type = TRet;
    using arg_types = std::tuple<Args...>;
    typedef TRet(TObject::*function_type)(Args...) const;

    static constexpr std::size_t arity = sizeof... (Args);
    static constexpr bool is_const = true;

    template <std::size_t N>
    struct argument
    {
        static_assert(N < arity, "error: invalid parameter index.");
        using type = typename std::tuple_element<N, std::tuple<Args...>>::type;
    };

    template <typename... TestArgs>
    static constexpr bool is_same_args = std::is_same_v<std::tuple<Args...>, std::tuple<TestArgs...>>;
};

/// Function and static member function traits.
template <typename TRet, typename... Args>
struct function_traits<TRet(*)(Args...)>
{
    using return_type = TRet;
    using arg_types = std::tuple<Args...>;
    typedef TRet(*function_type)(Args...);

    static constexpr std::size_t arity = sizeof... (Args);
    static constexpr bool is_const = false;

    template <std::size_t N>
    struct argument
    {
        static_assert(N < arity, "error: invalid parameter index.");
        using type = typename std::tuple_element<N, std::tuple<Args...>>::type;
    };

    template <typename... TestArgs>
    static constexpr bool is_same_args = std::is_same_v<std::tuple<Args...>, std::tuple<TestArgs...>>;
};

template <typename Function, typename ArgType, std::size_t N>
class is_same_arg
{
    template <typename> static std::false_type test(...);
    static constexpr auto test()
    {
        if constexpr (function_traits<Function>::arity > 0u)
        {
            return std::is_same_v<typename function_traits<Function>::template argument<N>::type, ArgType>;
        }
        else
        {
            return false;
        }
    }

public:
    static constexpr bool value = test();
};

} // namespace traits

#endif // WRAP_FUNCTION_TRAITS_HPP
