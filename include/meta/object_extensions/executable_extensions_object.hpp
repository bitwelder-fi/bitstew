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

#ifndef META_EXECUTABLE_EXTENSIONS_OBJECT_HPP
#define META_EXECUTABLE_EXTENSIONS_OBJECT_HPP

#include <meta/arguments/packaged_arguments.hpp>

#include <memory>
#include <string_view>
#include <unordered_map>

namespace meta
{

class ExecutableExtension;
using ExecutableExtensionPtr = std::shared_ptr<ExecutableExtension>;

class ExecutableExtensionsObject;
using ExecutableExtensionsObjectPtr = std::shared_ptr<ExecutableExtensionsObject>;

/// Base class of objects which support callable extensions.
/// \see Object
class META_API ExecutableExtensionsObject
{
public:
    virtual ~ExecutableExtensionsObject();

    /// Adds an extension to the object. The object takes ownership over the extension. The method
    /// fails if the extension is already added to the object.
    /// \param extension The extension to add to the object.
    /// \return If the extension gets added with success, returns \e true, otherwise \e false.
    void addExtension(ExecutableExtensionPtr extension);

    /// Removes an extension from the object. The extension gets destroyed if the object owns the
    /// extension. The method fails if the extension does not extend the object.
    /// \param extension The extension to remove from the object.
    /// \return If the extension gets removed with success, returns \e true, otherwise \e false.
    bool removeExtension(ExecutableExtension& extension);

    /// Tries to locate the extension with the name.
    /// \param name The extension name to locate.
    /// \return The extension with the name, or \e nullptr if the object has no extension with the
    ///         name registered.
    ExecutableExtensionPtr findExtension(std::string_view name) const;

    /// Invokes an extension of the object.
    /// \param name The name of the object extension to invoke.
    /// \param args Optional, the arguments with which to invoke the extension.
    /// \return returns one of the following:
    ///         - If the extension is found, and has a return value, the return value of the extension.
    ///         - If the extension is found, and has no return value, returns an invalid Argument.
    ///         - If the extension is not found, returns nullopt.
    ReturnValue invoke(std::string_view name, PackagedArguments args = PackagedArguments());

    template <typename... Arguments>
    ReturnValue invoke(std::string_view name, Arguments... arguments)
    {
        return ExecutableExtensionsObject::invoke(name, PackagedArguments(arguments...));
    }

protected:
    void initialize();

private:
    using ExtensionsMap = std::unordered_map<std::string_view, ExecutableExtensionPtr>;

    ExtensionsMap m_extensions;
};

/// Invokes an extension of a raw pointer object.
/// \param object The object whose extension to invoke.
/// \param name The extension name to invoke.
/// \param arguments The arguments with which to invoke the extension.
/// \return returns one of the following:
///         - If the extension is found, and has a return value, the return value of the extension.
///         - If the extension is found, and has no return value, returns an invalid Argument.
///         - If the extension is not found, returns nullopt.
META_API ReturnValue invoke(ExecutableExtensionsObject* object, std::string_view name, PackagedArguments arguments = PackagedArguments());

/// Invokes an extension of a object.
/// \param object The object whose extension to invoke.
/// \param name The extension name to invoke.
/// \param arguments The arguments with which to invoke the extension.
/// \return returns one of the following:
///         - If the extension is found, and has a return value, the return value of the extension.
///         - If the extension is found, and has no return value, returns an invalid Argument.
///         - If the extension is not found, returns nullopt.
META_API ReturnValue invoke(ExecutableExtensionsObjectPtr object, std::string_view name, PackagedArguments arguments = PackagedArguments());

}

#endif // META_EXECUTABLE_EXTENSIONS_OBJECT_HPP
