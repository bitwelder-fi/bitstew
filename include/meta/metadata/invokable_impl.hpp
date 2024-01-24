/*
 * Copyright (C) 2023=4 bitWelder
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

#include <utils/function_traits.hpp>
#include <meta/metadata/invokable.hpp>

namespace meta
{

namespace detail
{

template <typename Function, typename ArgType, std::size_t N>
class enableRepack
{
    template <typename> static std::false_type test(...);
    template <typename U> static auto test(int)
    {
        if constexpr (traits::function_traits<U>::arity > 0u)
        {
            return std::is_same_v<typename traits::function_traits<U>::template argument<N>::type, ArgType>;
        }
        else
        {
            return false;
        }
    }

public:
    static constexpr bool packObject = std::is_member_function_pointer_v<Function>;
    static constexpr bool packSelf = std::is_same<decltype(test<Function>(0)), std::true_type>::value;
    static constexpr bool value = packObject || packSelf;
};

}

template <class Function>
Invokable::InvokableDescriptor<Function>::InvokableDescriptor(std::string_view name, Function function) :
    ObjectExtension::Descriptor(name, detail::enableRepack<Function, Invokable*, 0u>::value),
    function(function)
{
}

template <class Function>
PackagedArguments Invokable::InvokableDescriptor<Function>::repackArguments(const PackagedArguments& arguments)
{
    auto result = PackagedArguments();

    if constexpr (detail::enableRepack<Function, Invokable*, 0u>::packObject)
    {
        using ClassType = typename traits::function_traits<Function>::object;
        if constexpr (std::is_base_of_v<Object, ClassType>)
        {
            auto object = this->owner.lock();
            if (object)
            {
                result += ArgumentData(dynamic_cast<ClassType*>(object.get()));
            }
        }
    }
    if constexpr (detail::enableRepack<Function, Invokable*, 0u>::packSelf)
    {
        result += ArgumentData(this);
    }

    result += arguments;

    return result;
}

template <class Function>
ArgumentData Invokable::InvokableDescriptor<Function>::execute(const PackagedArguments& arguments)
{
    try
    {
        auto pack = arguments.toTuple<Function>();
        if constexpr (std::is_void_v<typename traits::function_traits<Function>::return_type>)
        {
            std::apply(function, pack);
            return ArgumentData();
        }
        else
        {
            auto result = std::apply(function, pack);
            return ArgumentData(result);
        }
    }
    catch (const std::exception& e)
    {
        META_LOG_ERROR(e.what());
        return ArgumentData();
    }
}

}
