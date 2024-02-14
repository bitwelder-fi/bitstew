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

#include <meta/object_extensions/object_extension.hpp>
#include <meta/object_extensions/signal.hpp>
#include <meta/meta.hpp>
#include <meta/object.hpp>

#include <chrono>

namespace meta
{

Connection::Connection(ObjectExtension& signal, ObjectExtension& slot) :
    m_signal(signal.weak_from_this()),
    m_slot(slot.weak_from_this()),
    m_id(std::chrono::steady_clock::now().time_since_epoch().count())
{
}

bool Connection::isValid() const
{
    return m_signal.lock() && m_slot.lock();
}

ObjectExtensionPtr Connection::getSource() const
{
    return m_signal.lock();
}

ObjectExtensionPtr Connection::getTarget() const
{
    return m_slot.lock();
}

bool operator==(const Connection& lhs, const Connection& rhs)
{
    return lhs.m_id == rhs.m_id;
}

bool operator<(const Connection& lhs, const Connection& rhs)
{
    return lhs.m_id < rhs.m_id;
}

bool operator>(const Connection& lhs, const Connection& rhs)
{
    return lhs.m_id > rhs.m_id;
}


ObjectExtension::ObjectExtension(std::string_view name) :
    MetaObject(name)
{
}

void ObjectExtension::attachToObject(Object& object)
{
    abortIfFail(!m_object.lock());

    m_object = object.weak_from_this();
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

ReturnValue ObjectExtension::run(const PackagedArguments& arguments)
{
    return runOverride(arguments);
}

void ObjectExtension::addConnection(const Connection& connection)
{
    abortIfFail(findConnection(connection) == m_connections.end());

    m_connections.push_back(connection);
}

void ObjectExtension::removeConnection(const Connection& connection)
{
    auto it = findConnection(connection);
    abortIfFail(it != m_connections.end());

    m_connections.erase(it);
}

ObjectExtension::ConnectionContainer::iterator ObjectExtension::findConnection(const Connection& connection)
{
    return std::find(m_connections.begin(), m_connections.end(), connection);
}

void ObjectExtension::compactConnections()
{
    std::erase_if(m_connections, [](auto& connection) { return !connection.isValid(); });
}

}
