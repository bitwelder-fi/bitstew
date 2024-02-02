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
/// call invokable extensions by calling the meta::invoke() function.
///
/// In most cases, the invokable needs to access the instance it extends. To do that, the first argument
/// of the invokable function should be a pionter to ObjectExtension:
/// \code
/// auto lambda = [](meta::ObjectExtension* self)
/// {
///     // You can access the object through the extension.
///     auto object = self->getOwner();
/// };
/// using LambdaExtension = meta::Invokable<decltype(lambda), lambda>;
/// object->addExtension(LambdaExtension::create("lambda"));
/// \endcode
///
/// The invokable arguments can hold any type, except reference types.
template <class Function, Function function>
class Invokable final : public ObjectExtension
{
    using SelfType = Invokable<Function, function>;

protected:
    /// Repackages the arguments, appending the owning object and itself, when required.
    PackagedArguments repackageArguments(const PackagedArguments& arguments);
    /// Overrides ObjectExtension::Descriptor::runOverride().
    ArgumentData runOverride(const PackagedArguments& arguments);

    /// Constructor.
    explicit Invokable(std::string_view name);

public:
    STUB_META_CLASS(SelfType, ObjectExtension)
    {
    };

    /// Creates an instance of the Invokable type.
    /// \param name The name of the invokable.
    static auto create(std::string_view name, const PackagedArguments& = PackagedArguments())
    {
        return std::shared_ptr<SelfType>(new SelfType(name));
    }
};

}

#include <meta/metadata/invokable_impl.hpp>

#endif // META_INVOKABLE_HPP
