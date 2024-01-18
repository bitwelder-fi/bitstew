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

#ifndef META_METAOBJECT_HPP
#define META_METAOBJECT_HPP

#include <meta/meta_api.hpp>
#include <meta/metadata/metaclass.hpp>
#include <meta/metadata/invokable.hpp>

#include <memory>
#include <optional>

namespace meta
{

class Object;
using ObjectPtr = std::shared_ptr<Object>;


/// The base class of any object that defines a meta class.
class META_API Object
{
public:
    /// Creates a meta-object.
    static ObjectPtr create(std::string_view name);

    /// Destructor.
    virtual ~Object() = default;

    /// Returns the name of the object.
    /// \return The name of the object.
    std::string_view getName() const
    {
        return m_name;
    }

    /// The metadata of a meta object.
    META_CLASS("meta.Object", Object)
    {
        MetaInvokable _getName{*this, "getName", &Object::getName};
    };

protected:
    /// Constructor.
    explicit Object(std::string_view name);

private:
    const std::string m_name;
};

/// Invokes an invokable of a metaobject. The invokable is called from the metaclass of the metaobject.
/// \param object The metaobject whose metamethod to call.
/// \param invokableName The invokable name to call.
/// \param arguments The arguments with which to invoke the metamethod.
/// \return returns one of the following:
///         - If the invokable is found, and has a return value, the return value of the invokable.
///         - If the invokable is found, and has no return value, returns an invalid ArgumentData.
///         - If the invokable is not found, returns nullopt.
META_API std::optional<ArgumentData> invoke(ObjectPtr object, std::string_view invokableName, const PackagedArguments& arguments);

}

#endif // META_METAOBJECT_HPP
