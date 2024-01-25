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

#ifndef META_OBJECT_EXTENSION_HPP
#define META_OBJECT_EXTENSION_HPP

#include <meta/arguments/argument_type.hpp>
#include <meta/forwards.hpp>
#include <meta/meta_api.hpp>
#include <meta/metadata/metaclass.hpp>

#include <pimpl.hpp>

namespace meta
{

/// The base class of object extensions. Object extensions dynamically extend an instance of an Object
/// added functionality.
/// You can define object extensions on a metaclass. When defined on those, the metaclass factory will
/// add all the extensions to the instance it creates.
class META_API ObjectExtension : public MetaObject, public std::enable_shared_from_this<ObjectExtension>
{
public:
    /// Destructor.
    virtual ~ObjectExtension() = default;

    /// Returns the object which owns the object extension.
    /// \return The object which owns the object extension.
    ObjectPtr getOwner() const;

    /// The entry point of an object extensions.
    /// \param arguments The arguments with which the extension gets executed.
    /// \return The return value of the extension execution. Extensions which do not return any value
    ///         return a void ArgumentData.
    ArgumentData execute(const PackagedArguments& arguments = PackagedArguments());

    /// The metaclass of the object extension.
    // META_CLASS("meta.ObjectExtension", ObjectExtension)
    // {
    // };

protected:
    /// The descriptor of an object extension.
    struct META_API Descriptor
    {
        ObjectWeakPtr owner;
        bool repack = false;

        explicit Descriptor(bool repack) :
            repack(repack)
        {
        }
        virtual ~Descriptor() = default;

        /// Override this to provide extension specific executor.
        /// \param arguments The arguments with which the extension gets executed.
        /// \return The return value of the extension execution. Extensions which do not return any value
        ///         return a void ArgumentData.
        virtual ArgumentData execute(const PackagedArguments& arguments) = 0;

        /// Override this if your object extension requires arguments repackaging.
        /// \param arguments The arguments to repackage.
        /// \return The repackaged arguments.
        virtual PackagedArguments repackageArguments(const PackagedArguments& arguments);
    };

    /// Constructor, creates an object extension with a descriptor passed as argument.
    explicit ObjectExtension(std::string_view name, pimpl::d_ptr_type<Descriptor> descriptor);

    /// Meta calls this method when an object extension gets attached to an Object.
    virtual void onAttached()
    {
    }

    /// Meta calls this method when an object extension gets detached from the Object to which is
    /// attached.
    virtual void onDetached()
    {
    }

    DECLARE_DESCRIPTOR(Descriptor, m_descriptor)

private:
    DISABLE_COPY(ObjectExtension);
    DISABLE_MOVE(ObjectExtension);

    pimpl::d_ptr_type<Descriptor> m_descriptor;
    friend struct ObjectDescriptor;
};

}

#endif // META_OBJECT_EXTENSION_HPP
