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

#ifndef STEW_CONNECTION_HPP
#define STEW_CONNECTION_HPP

#include <stew/dynamic_type/packaged_arguments.hpp>
#include <stew/forwards.hpp>
#include <stew/stew_api.hpp>

namespace stew
{

/// The %Connection defines a connection token between two object extensions. Signal uses this to
/// identify a slot connected to a signal.
struct STEW_API Connection : public std::enable_shared_from_this<Connection>
{
    /// Creates a connection with a source and a target.
    static ConnectionPtr create(ObjectExtension& source, ObjectExtension& target);
    /// Returns whether the connection is valid. A connection is valid if both source and target are
    /// defined.
    /// \return If the connection is valid, returns \e true, otherwise \e false.
    bool isValid() const;

    /// Returns the source object extension of the connection.
    /// \return The source object extension of a valid connection, or an invalid object extension if
    ///         the connection is invalid.
    ObjectExtensionPtr getSource() const;

    template <typename T>
    auto getSource() const
    {
        return std::dynamic_pointer_cast<T>(Connection::getSource());
    }

    /// Returns the target object extension of the connection.
    /// \return The target object extension of a valid connection, or an invalid object extension if
    ///         the connection is invalid.
    ObjectExtensionPtr getTarget() const;

    template <typename T>
    auto getTarget() const
    {
        return std::dynamic_pointer_cast<T>(Connection::getTarget());
    }

private:
    friend class ObjectExtension;

    DISABLE_COPY(Connection);

    /// Constructor, creates a connection with a source and a target.
    explicit Connection(ObjectExtension& source, ObjectExtension& target);

    /// Resets the connection.
    void reset();

    ObjectExtensionWeakPtr m_source;
    ObjectExtensionWeakPtr m_target;
};

}

#endif
