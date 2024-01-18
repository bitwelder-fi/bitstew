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

#ifndef META_INVOKABLE_HPP
#define META_INVOKABLE_HPP

#include <meta/meta_api.hpp>
#include <meta/arguments/argument_type.hpp>

#include <functional>
#include <string_view>
#include <utils/function_traits.hpp>

namespace meta
{

/// Represents an invokable object. An invokable can be a function, a lambda, a functor or a method.
///
/// The invokable arguments can hold any type, except reference types.
class META_API Invokable
{
public:
    /// The default constructor.
    explicit Invokable() = default;

    /// Creates an invokable object for a function, a functor, a lambda or a method.
    /// \tparam Function The function type.
    /// \param name The name of the function.
    /// \param function The function of the invokable.
    template<class Function>
    explicit Invokable(std::string_view name, Function function);

    /// Move constructor.
    Invokable(Invokable&& other);
    /// Move operator.
    Invokable& operator=(Invokable&& other);
    /// Swaps this invokable with \a other.
    /// \param other The invokable with which to swap this invokable.
    void swap(Invokable& other);

    /// Invokes the invokable with packaged arguments.
    /// \param arguments The packaged arguments with which to invoke the invokable.
    /// \return The return value of the invokable. If the invokable is void, returns an invalid ArgumentData.
    ArgumentData apply(const PackagedArguments& arguments)
    {
        return m_descriptor.invokable(arguments);
    }

    /// Invokes the invokable with an object and packaged arguments.
    /// \param arguments The arguments with which to invoke the invokable.
    /// \return The return value of the invokable. If the invokable is void, returns an invalid ArgumentData.
    template <class ClassType>
    ArgumentData apply(ClassType* object, const PackagedArguments& arguments)
    {
        return m_descriptor.invokable(PackagedArguments(object).append(arguments));
    }

    /// Returns the name of the invokable.
    /// \return The name of the invokable.
    std::string_view getName() const
    {
        return m_descriptor.name;
    }

    /// Returns whether the invokable is valid.
    bool isValid() const;

protected:
    DISABLE_COPY(Invokable);
    using InvokableFunction = std::function<ArgumentData(const PackagedArguments&)>;

    /// The descriptor type of the invokable.
    struct META_API Descriptor
    {
        /// The invokable function of the invokable object.
        InvokableFunction invokable;
        /// The name of the invokable object.
        std::string name;
    };

    /// The descriptor of the invokable.
    Descriptor m_descriptor;
};

}

#include <meta/metadata/invokable_impl.hpp>

#endif // META_INVOKABLE_HPP
