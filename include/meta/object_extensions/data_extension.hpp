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

#ifndef META_DATA_EXTENSION_HPP
#define META_DATA_EXTENSION_HPP

#include <meta/arguments/argument.hpp>
#include <meta/metadata/metaclass.hpp>
#include <meta/metadata/meta_object.hpp>
#include <meta/object_extensions/invokable.hpp>
#include <meta/object_extensions/signal.hpp>

#include <memory>

namespace meta
{

class DataExtension;
using DataExtensionPtr = std::shared_ptr<DataExtension>;

class META_API DataExtension : public MetaObject, public std::enable_shared_from_this<DataExtension>
{
protected:
    explicit DataExtension(std::string_view name);

    virtual Argument get() const = 0;
    virtual void set(Argument& value) = 0;

public:
    META_CLASS("meta.DataExtension", DataExtension, MetaObject)
    {
        DECLARE_INVOKABLE(Get, "get", &DataExtension::get);
        DECLARE_INVOKABLE(Set, "set", &DataExtension::set);

        META_EXTENSION(Get);
        META_EXTENSION(Set);
    };
};

}

#endif // META_DATA_EXTENSION_HPP
