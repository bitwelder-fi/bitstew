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

#ifndef META_DATA_EXTENSIONS_OBJECT_HPP
#define META_DATA_EXTENSIONS_OBJECT_HPP

#include <meta/arguments/packaged_arguments.hpp>
#include <meta/forwards.hpp>

#include <memory>

namespace meta
{

class DataExtensionsObject;
using DataExtensionsObjectPtr = std::shared_ptr<DataExtensionsObject>;

class META_API DataExtensionsObject
{
public:
    virtual ~DataExtensionsObject();

    /// Adds a data extension to the object. The object takes ownership over the extension. The method
    /// fails if the extension is already added to the object.
    /// \param data The data extension to add to the object.
    /// \return If the extension gets added with success, returns \e true, otherwise \e false.
    void addData(DataExtensionPtr data);

    /// Removes a data extension from the object. The extension gets destroyed if the object owns the
    /// extension. The method fails if the extension does not extend the object.
    /// \param data The data extension to remove from the object.
    /// \return If the extension gets removed with success, returns \e true, otherwise \e false.
    bool removeData(DataExtension& data);

    /// Tries to locate the data extension with the name.
    /// \param name The extension name to locate.
    /// \return The extension with the name, or \e nullptr if the object has no extension with the
    ///         name registered.
    DataExtensionPtr findData(std::string_view name) const;
};

}

#endif // META_DATA_EXTENSIONS_OBJECT_HPP
