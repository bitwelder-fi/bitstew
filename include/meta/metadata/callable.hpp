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

#ifndef META_CALLABLE_HPP
#define META_CALLABLE_HPP

#include <meta/meta_api.hpp>
#include <meta/arguments/argument_type.hpp>

#include <functional>
#include <string_view>
#include <optional>
#include <utils/function_traits.hpp>

namespace meta
{

/// Represents a callable. A function, a lambda, a function or a method.
///
/// The callable arguments can hold any type, except reference types.
class META_API Callable
{
public:
    /// The default constructor.
    explicit Callable() = default;

    /// Creates a callable object for a function, a functor, a lambda or a method.
    /// \tparam Function The function type.
    /// \param name The name of the function.
    /// \param function The function of the callable.
    template<class Function>
    explicit Callable(std::string_view name, Function function);

    /// Move constructor.
    Callable(Callable&& other);
    /// Move operator.
    Callable& operator=(Callable&& other);
    /// Swaps this callable with \a other.
    /// \param other The callable with which to swap this callable.
    void swap(Callable& other);

    /// Invokes the callable with packaged arguments.
    /// \param arguments The packaged arguments with which to invoke the callable.
    /// \return The return value of the callable. If the callable is void, returns \e nullopt.
    std::optional<ArgumentData> apply(const PackagedArguments& arguments)
    {
        return m_descriptor.invokable(arguments);
    }

    /// Invokes the callable with an object and packaged arguments.
    /// \param arguments The arguments with which to invoke the callable.
    /// \return The return value of the callable. If the callable is void, returns \e nullopt.
    template <class ClassType>
    std::optional<ArgumentData> apply(ClassType* object, const PackagedArguments& arguments)
    {
        return m_descriptor.invokable(PackagedArguments(object).append(arguments));
    }

    /// Returns the name of the callable.
    /// \return The name of the callable.
    std::string_view getName() const
    {
        return m_descriptor.name;
    }

    /// Returns whether the callable is valid.
    bool isValid() const;

protected:
    DISABLE_COPY(Callable);
    using Invokable = std::function<std::optional<ArgumentData>(const PackagedArguments&)>;

    /// The descriptor type of the callable.
    struct META_API CallableDescriptor
    {
        /// The invokable function of the callable object.
        Invokable invokable;
        /// The name of the callable object.
        std::string name;
    };

    /// The descriptor of the callable.
    CallableDescriptor m_descriptor;
};

}

#include <meta/metadata/callable_impl.hpp>

#endif // META_CALLABLE_HPP
