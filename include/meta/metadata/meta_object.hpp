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

#ifndef META_OBJECT_HPP
#define META_OBJECT_HPP

#include <meta/meta.hpp>
#include <meta/metadata/metaclass.hpp>

#include <memory>

namespace meta
{

/// The base class of objects whith metadata.
class META_API MetaObject
{
public:
    /// Destructor.
    virtual ~MetaObject();

    /// Returns the name of the metaobject.
    /// \return The name of the metaobject.
    std::string_view getName() const
    {
        return m_name;
    }

    META_CLASS("meta.MetaObject", MetaObject)
    {
    };

    static MetaObjectPtr create(std::string_view, const PackagedArguments&)
    {
        return {};
    }

protected:
    /// Constructor. Fails if the metaname passed as argument is invalid.
    explicit MetaObject(std::string_view metaName);

private:
    /// The metaname of the metaobject.
    std::string m_name;
};

}

#endif
