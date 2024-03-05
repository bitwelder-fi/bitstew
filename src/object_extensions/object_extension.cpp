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

#include <meta/object_extensions/connection.hpp>
#include <meta/object_extensions/object_extension.hpp>
#include <meta/object_extensions/signal.hpp>
#include <meta/meta.hpp>
#include <meta/object.hpp>
#include <utils/container_view.hpp>

namespace meta
{

Connection::Connection(ObjectExtension& source, ObjectExtension& target) :
    m_source(source.weak_from_this()),
    m_target(target.weak_from_this())
{
}

ConnectionPtr Connection::create(ObjectExtension& source, ObjectExtension& target)
{
    return ConnectionPtr(new Connection(source, target));
}

bool Connection::isValid() const
{
    return m_source.lock() && m_target.lock();
}

ObjectExtensionPtr Connection::getSource() const
{
    return m_source.lock();
}

ObjectExtensionPtr Connection::getTarget() const
{
    return m_target.lock();
}

void Connection::reset()
{
    m_target.reset();
    m_source.reset();
}


ObjectExtension::ObjectExtension(std::string_view name) :
    MetaObject(name)
{
}

void ObjectExtension::attachToObject(Object& object)
{
    abortIfFail(!m_object.lock());

    m_object = object.weak_from_this();
    if (m_object.expired())
    {
        return;
    }

    onAttached();
}

void ObjectExtension::detachFromObject()
{
    abortIfFail(m_object.lock());
    onDetached();

    m_object.reset();
}


ObjectPtr ObjectExtension::getObject() const
{
    return m_object.lock();
}

ReturnValue ObjectExtension::run(PackagedArguments arguments)
{
    if (m_connections.isLocked())
    {
        return {};
    }

    // Make sure the extension is alive till it is running.
    auto keepAlive = shared_from_this();
    ReturnValue result;

    {
        utils::ContainerView<ConnectionContainer> guard(m_connections);
        result = runOverride(arguments);
    }

    return result;
}

void ObjectExtension::addConnection(ConnectionPtr connection)
{
    // Add the connection to both source and target. It should be called on source!
    abortIfFail(connection->getSource().get() == this);
    abortIfFail(!findConnection(*connection));

    m_connections.push_back(connection);
    connection->getTarget()->m_connections.push_back(connection);
}

void ObjectExtension::removeConnection(ConnectionPtr connection)
{
    // Remove the connection from both source and target.
    auto pos = findConnection(*connection);
    abortIfFail(pos);
    // The method should be called on source!
    abortIfFail((**pos)->getSource().get() == this);

    m_connections.erase(*pos);

    // Go to target, and remove the connection from there too.
    auto target = connection->getTarget();
    if (!target)
    {
        return;
    }

    pos = target->findConnection(*connection);
    abortIfFail(pos);
    target->m_connections.erase(*pos);

    connection->reset();
}

void ObjectExtension::disconnectTarget()
{
    utils::ContainerView<ConnectionContainer> guard(m_connections);
    auto self = shared_from_this();
    // The disconnect affects the whole range, so ensure that we use the full connection range, not
    // only the locked.
    for (auto connection : m_connections.getView())
    {
        if (connection->getTarget() == self)
        {
            auto source = connection->getSource();
            if (!source)
            {
                continue;
            }

            source->removeConnection(connection);
        }
    }
}

std::optional<ObjectExtension::ConnectionContainer::Iterator> ObjectExtension::findConnection(Connection& connection)
{
    auto range = m_connections.getView();
    return range.find(connection.shared_from_this());
}

}
