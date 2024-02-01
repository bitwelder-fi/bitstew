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

/// %InvokableType represents an invokable extension of an Object, which can be applied on an instance
/// at runtime. To add an invokable to an instance of an Object, call Object::addExtension(). You can
/// invoke invokable extensions by calling the meta::invoke() function.
///
/// When an invokable extension gets added to an instance of an Object, the instance takes the ownership
/// over the invokable. An invokable may only be attached to a single Object instance at a time. You
/// can move an invokable from an Object instance to an other by removing the invokable from the source
/// instance before adding it to the next instance.
///
/// The invokable arguments can hold any type, except reference types.
template <class Function, Function function>
class InvokableType final : public ObjectExtension
{
    using SelfType = InvokableType<Function, function>;

protected:
    PackagedArguments repackageArguments(const PackagedArguments& arguments);
    /// Overrides ObjectExtension::Descriptor::execute().
    ArgumentData executeOverride(const PackagedArguments& arguments);

    explicit InvokableType(std::string_view name);

public:
    STUB_META_CLASS(SelfType, ObjectExtension)
    {
    };

    static auto create(std::string_view name, const PackagedArguments& = PackagedArguments())
    {
        return std::shared_ptr<SelfType>(new SelfType(name));
    }
};

}

#include <meta/metadata/invokable_impl.hpp>

#endif // META_INVOKABLE_HPP
