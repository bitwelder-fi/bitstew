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

#include <meta/arguments/packaged_arguments.hpp>
#include <meta/forwards.hpp>
#include <meta/meta_api.hpp>
#include <meta/metadata/meta_object.hpp>

#include <pimpl.hpp>

namespace meta
{

/// The base class of object extensions. Object extensions dynamically extend an instance of an Object
/// added functionality.
///
/// When you add an object extension to an instance of an Object, the instance takes the ownership
/// over the object extension. An object extension may only be attached to a single Object instance
/// at a time. You can move an object extension from an Object instance to an other by removing the
/// object extension from the source instance before adding it to the destination instance.
///
/// You can add object extensions to a metaclass. When you instantiate an object through the meta class,
/// all the extensions added to the meta class will be instantiated and added to the instance.
class META_API ObjectExtension : public MetaObject, public std::enable_shared_from_this<ObjectExtension>
{
public:
    /// Destructor.
    virtual ~ObjectExtension() = default;

    /// Returns the object which owns the object extension.
    /// \return The object which owns the object extension.
    ObjectPtr getObject() const;

    /// The entry point of an object extensions.
    /// \param arguments The arguments with which the extension gets executed.
    /// \return The return value of the extension execution. Extensions which do not return any value
    ///         return a void Argument.
    Argument run(const PackagedArguments& arguments = PackagedArguments());

    /// The metaclass of the object extension.
    META_CLASS("meta.ObjectExtension", ObjectExtension, MetaObject)
    {
    };

    static ObjectExtensionPtr create(std::string_view, const PackagedArguments&)
    {
        return {};
    }

protected:
    /// Constructor, creates an object extension with a descriptor passed as argument.
    explicit ObjectExtension(std::string_view name);

    /// Override this to provide extension specific executor.
    /// \param arguments The arguments with which the extension gets executed.
    /// \return The return value of the extension execution. Extensions which do not return any value
    ///         return a void Argument.
    virtual Argument runOverride(const PackagedArguments& arguments) = 0;

    /// Meta calls this method when an object extension gets attached to an Object.
    virtual void onAttached()
    {
    }

    /// Meta calls this method when an object extension gets detached from the Object to which is
    /// attached.
    virtual void onDetached()
    {
    }

private:
    DISABLE_COPY(ObjectExtension);
    DISABLE_MOVE(ObjectExtension);

    void attachToObject(Object& object);
    void detachFromObject();

    ObjectWeakPtr m_object;
    friend class Object;
};

}

#endif // META_OBJECT_EXTENSION_HPP
