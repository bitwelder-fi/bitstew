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

#ifndef META_INVOKABLE_HPP
#define META_INVOKABLE_HPP

#include <meta/arguments/argument_type.hpp>
#include <meta/forwards.hpp>
#include <meta/meta_api.hpp>
#include <meta/object_extension.hpp>

#include <string_view>

namespace meta
{

/// %Invokable represents an invokable extension of an Object, which can be applied on an instance
/// at runtime. To add an invokable to an instance of an Object, call Object::addExtension(). You can
/// invoke invokable extensions by calling the meta::invoke() function.
///
/// When an invokable extension gets added to an instance of an Object, the instance takes the ownership
/// over the invokable. An invokable may only be attached to a single Object instance at a time. You
/// can move an invokable from an Object instance to an other by removing the invokable from the source
/// instance before adding it to the next instance.
///
/// The invokable arguments can hold any type, except reference types.
class META_API Invokable final : public ObjectExtension
{
public:
    /// Creates an invokable object for a function, a functor, a lambda or a method. These invokables
    /// may get any types of arguments. When invoked, Meta checks whether the first argument type is
    /// Invokable type, and passes the invokable instance as argument.
    /// \tparam Function The function type.
    /// \param name The name of the function.
    /// \param function The function of the invokable.
    template<class Function>
    static InvokablePtr create(std::string_view name, Function function)
    {
        return std::shared_ptr<Invokable>(new Invokable(name, function));
    }

protected:
    DISABLE_COPY(Invokable);

    template<class Function>
    explicit Invokable(std::string_view name, Function function) :
        ObjectExtension(name, pimpl::make_d_ptr<InvokableDescriptor<Function>>(*this, function))
    {
    }

    /// The descriptor type of the invokable.
    template <class Function>
    struct META_API InvokableDescriptor final : ObjectExtension::Descriptor
    {
        /// The invokable which owns the descriptor.
        Invokable* invokable = nullptr;
        /// The invokable function.
        Function function;

        /// Constructor, creates a descriptor for a function.
        explicit InvokableDescriptor(Invokable& invokable, Function function);

        /// Overrides ObjectExtension::Descriptor::execute().
        ArgumentData execute(const PackagedArguments& arguments);

        /// Overrides ObjectExtension::Descriptor::execute().
        PackagedArguments repackageArguments(const PackagedArguments& arguments);
    };
};

}

#include <meta/metadata/invokable_impl.hpp>

#endif // META_INVOKABLE_HPP
