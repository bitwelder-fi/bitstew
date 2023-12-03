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

#include <memory>

namespace meta
{

class MetaObject;
using MetaObjectPtr = std::shared_ptr<MetaObject>;

/// The base class of any object that defines a meta class.
class META_API MetaObject
{
public:
    /// The metadata of a meta object.
    BaseMetaData(MetaObject)
    {
    };
    static MetaObjectPtr create(std::string_view name);

    /// Destructor.
    virtual ~MetaObject() = default;

    /// Returns the name of the object.
    /// \return The name of the object.
    std::string_view getName() const
    {
        return m_name;
    }

protected:
    /// Constructor.
    explicit MetaObject(std::string_view name);

private:
    const std::string m_name;
};

}

#endif // META_METAOBJECT_HPP
