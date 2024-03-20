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

#ifndef META_OBJECT_HPP
#define META_OBJECT_HPP

#include <meta/forwards.hpp>
#include <meta/metadata/meta_object.hpp>
#include <meta/object_extensions/executable_extensions_object.hpp>
#include <meta/object_extensions/data_extensions_object.hpp>

namespace meta
{

/// The base class of any object that defines a meta-class.
class META_API Object : public MetaObject,
                        public DataExtensionsObject,
                        public ExecutableExtensionsObject,
                        public std::enable_shared_from_this<Object>
{
public:
    /// Creates a meta-object.
    static ObjectPtr create(std::string_view name);

    /// Destructor.
    ~Object() override;

    /// The metadata of a meta object.
    META_CLASS("meta.Object", Object, MetaObject)
    {
    };

protected:
    /// Constructor.
    explicit Object(std::string_view name);

    /// Second phase initializer.
    void initialize();

};

}

#endif // META_METAOBJECT_HPP
