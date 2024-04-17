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

#include <stew/object.hpp>
#include <stew/object_extensions/connection.hpp>
#include <stew/object_extensions/signal.hpp>
#include <stew/containers/view.hpp>

namespace stew
{

SignalExtension::SignalExtension(std::string_view name) :
    ObjectExtension(name)
{
}

SignalExtension::~SignalExtension()
{
    abortIfFail(!m_connections.getRefCount());
}

ReturnValue SignalExtension::runOverride(PackagedArguments arguments)
{
    if (!verifySignature(arguments))
    {
        return {};
    }

    auto result = 0;
    // Loop through the connections, excluding eventual new connections which may occur during
    // slot activations. The connections should have a locked view already!

    containers::LockView<ConnectionContainer> view(m_connections);
    for (auto& connection : view)
    {
        if (!connection || !connection->isValid())
        {
            // Skip disconnected connections.
            continue;
        }
        // Keep the slot alive while running.
        auto slot = connection->getTarget();

        // Consider only those connections, where this signal is the source. Other connections are
        // those, where this signal is the slot.
        if (connection->getSource().get() != this || !slot)
        {
            continue;
        }

        utils::RelockGuard<ConnectionContainer> relock(m_connections);
        if (slot->run(arguments))
        {
            ++result;
        }
    }

    return result;
}

ConnectionPtr SignalExtension::connect(ObjectExtensionPtr slot)
{
    abortIfFail(slot);

    // Create a connection token and add to both this and slot object extensions.
    auto connection = Connection::create(*this, *slot);

    addConnection(connection);

    return connection;
}

ConnectionPtr SignalExtension::connect(std::string_view extensionName)
{
    auto object = getObject();
    abortIfFail(object);

    auto extension = object->findExtension(extensionName);
    if (!extension)
    {
        return {};
    }
    return connect(extension);
}

void SignalExtension::disconnect(Connection& connection)
{
    auto connectionPtr = connection.shared_from_this();
    removeConnection(connectionPtr);
}

bool SignalExtension::tryDisconnect()
{
    if (isTriggering())
    {
        return false;
    }

    ObjectExtension::disconnect();
    return true;
}

std::size_t SignalExtension::getConnectionCount() const
{
    if (isTriggering())
    {
        return m_connections.getGuardedView()->size();
    }

    ConnectionContainer::GuardedViewType view(m_connections.cbegin(), m_connections.cend());
    return view.size();
}

}
