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

#include <meta/object.hpp>
#include <meta/object_extensions/connection.hpp>
#include <meta/object_extensions/signal.hpp>
#include <utils/scope_value.hpp>

namespace meta
{

SignalExtension::SignalExtension(std::string_view name) :
    ObjectExtension(name)
{
}

ReturnValue SignalExtension::runOverride(const PackagedArguments& arguments)
{
    if (!verifySignature(arguments))
    {
        return {};
    }

    auto result = 0;
    // Loop through the connections, excluding eventual new connections which may occur during
    // slot activations.
    for (auto it = beginConnections(), end = endConnections(); it != end; ++it)
    {
        auto connection = *it;
        if (!connection || !connection->isValid())
        {
            // Skip disconnected connections.
            continue;
        }
        // Consider only those connections, where this signal is the source. Other connections are
        // those, where this signal is the slot.
        if (connection->getSource().get() != this)
        {
            continue;
        }

        // Keep the slot alive while running.
        auto slot = connection->getTarget();
        if (slot && slot->run(arguments))
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

void SignalExtension::disconnect(Connection& connection)
{
    auto connectionPtr = connection.shared_from_this();
    removeConnection(connectionPtr);
}

bool SignalExtension::tryReset()
{
    if (isTriggering())
    {
        return false;
    }


    while (beginConnections() != endConnections())
    {
        auto connection(*beginConnections());
        disconnect(*connection);
    }

    return true;
}

std::size_t SignalExtension::getConnectionCount() const
{
    if (isTriggering())
    {
        auto result = std::size_t(0u);
        for (auto it = beginConnections(), end = endConnections(); it != end; ++it)
        {
            if (*it && (*it)->isValid())
            {
                ++result;
            }
        }
        return result;
    }

    return std::distance(beginConnections(), endConnections());
}

}
