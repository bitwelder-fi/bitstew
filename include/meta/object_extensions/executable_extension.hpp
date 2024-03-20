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

#ifndef META_EXECUTABLE_EXTENSION_HPP
#define META_EXECUTABLE_EXTENSION_HPP

#include <meta/arguments/packaged_arguments.hpp>
#include <meta/forwards.hpp>
#include <meta/meta_api.hpp>
#include <meta/metadata/meta_object.hpp>
#include <containers/guarded_sequence_container.hpp>

#include <deque>

namespace meta
{

class ExecutableExtensionsObject;

/// The base class of callable object extensions. Callable object extensions at runtime extend an instance
/// of a ExecutableExtensionsObject with added functionality. To add an object extension to an object, call
/// ExecutableExtensionsObject::addExtension() method.
///
/// When you add a callable object extension to an object, the instance takes the ownership over the object
/// extension.
///
/// You can add executable extensions to a meta-class by adding its meta-data to that meta-class. You can
/// only add meta-data at runtime to an unsealed meta-class. See MetaClass on how to add a meta-data to a
/// static meta-class.
///
/// When you instantiate an object through the meta-class or through the Meta library factory, Meta adds all
/// the extensions to the instance.
class META_API ExecutableExtension : public MetaObject, public std::enable_shared_from_this<ExecutableExtension>
{
public:
    /// The metaclass of the object extension.
    META_CLASS("meta.ExecutableExtension", ExecutableExtension, MetaObject)
    {
    };

    /// Destructor.
    ~ExecutableExtension() override;

    /// The entry point of an object extensions. The method is not re-entrant, a recoursive call returns with
    /// failure.
    ///
    /// \param arguments The arguments with which the extension gets executed.
    /// \return The return value of the extension execution. Extensions which do not return any value
    ///         return a void Argument. On failure, returns a \e nullopt
    ReturnValue run(PackagedArguments arguments = PackagedArguments());

    /// Returns the object which owns the object extension.
    /// \return The object which owns the object extension.
    ExecutableExtensionsObject* getObject() const;

    /// Disconnects all connections where this object extension is set as target.
    void disconnectTarget();

    /// Disconnects all connections to and from this object.
    void disconnect();

protected:
    /// The container of the connections.
    using ConnectionContainer = containers::GuardedSequenceContainer<std::deque<ConnectionPtr>,
                                                                     [](const ConnectionPtr& connection) { return connection != nullptr; }>;

    /// Constructor, creates an object extension with a descriptor passed as argument.
    explicit ExecutableExtension(std::string_view name);

    /// Override this to provide extension specific executor.
    /// \param arguments The arguments with which the extension gets executed.
    /// \return The return value of the extension execution. Extensions which do not return any value
    ///         return a void Argument. On failure, implementations are expected to return \e nullopt.
    virtual ReturnValue runOverride(PackagedArguments arguments) = 0;

    /// Returns the iterator to the connection. Not thread safe!
    /// \param connection The connection to look for.
    /// \return If the connection is valid, and is found, returns the iterator to the connection. On
    ///         failure, returns the end iterator of the connection container.
    std::optional<ConnectionContainer::Iterator> findConnection(Connection& connecton);

    /// Adds a connection to both source and target object extensions. The method fails if the connection
    /// has already been added to the object extensions. The method must be called on source extension.
    /// \param connection The connectiomn to add to the extension.
    void addConnection(ConnectionPtr connection);

    /// Removes a connection from both source and target object extensions. The method fails if the
    /// connection is not found in the object extensions. The method must be called on source extension.
    /// \param connection The connectiomn to add to the extension.
    void removeConnection(ConnectionPtr connection);

    /// Meta calls this method when an object extension gets attached to an Object.
    virtual void onAttached()
    {
    }

    /// Meta calls this method when an object extension gets detached from the Object to which is
    /// attached.
    virtual void onDetached()
    {
    }

    /// The container with the connections.
    ConnectionContainer m_connections;

private:
    DISABLE_COPY(ExecutableExtension);
    DISABLE_MOVE(ExecutableExtension);

    void attachToObject(ExecutableExtensionsObject& object);
    void detachFromObject();

    ExecutableExtensionsObject* m_object = nullptr;
    friend class ExecutableExtensionsObject;
};


/// Executable Extension for a particular object type.
template <class ExtendedObjectType>
class META_TEMPLATE_API ExecutableExtensionImpl : public ExecutableExtension
{
    static_assert(std::is_base_of_v<ExecutableExtensionsObject, ExtendedObjectType>, "ExtendedObjectType is not an executable extensions object.");

public:
    /// The metaclass of the object extension.
    META_CLASS("meta.ExecutableExtension", ExecutableExtensionImpl<ExtendedObjectType>, ExecutableExtension)
    {
    };

    ExtendedObjectType* getObject() const
    {
        return static_cast<ExtendedObjectType*>(ExecutableExtension::getObject());
    }

protected:
    /// Constructor, creates an object extension with a descriptor passed as argument.
    explicit ExecutableExtensionImpl(std::string_view name) :
        ExecutableExtension(name)
    {
    }
};

}

#endif // META_EXECUTABLE_EXTENSION_HPP
