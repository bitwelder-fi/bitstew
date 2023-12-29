/*
 * Copyright (C) 2023 bitWelder
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

#include <meta/signal/slot.hpp>
#include <meta/signal/signal.hpp>
#include <utils/scope_value.hpp>
#include "signal_private.h"
#include <assert.hpp>

namespace meta
{

Signal::Signal(std::string_view name) :
    m_name(name)
{
    abortIfFail(isValidMetaName(m_name));
}

bool Signal::isValid() const
{
    return true;
}

int Signal::activateSlots(const PackagedArguments& arguments)
{
    auto result = 0;

    {
        utils::ScopeValue<bool> emitGuard(m_emitting, true);
        auto last = m_slots.end();
        for (auto it = m_slots.begin(); it != last; ++it)
        {
            if (!(*it) || (*it)->getSignal() != this)
            {
                continue;
            }

            if ((*it)->activate(arguments))
            {
                ++result;
            }
        }
    }

    // Compact disconnected connections.
    std::erase(m_slots, SlotPtr());

    return result;
}

int Signal::emit(const PackagedArguments& arguments)
{
    if (!verifySignature(arguments))
    {
        return -1;
    }

    return activateSlots(arguments);
}

Slot* Signal::connect(SlotPtr connection)
{
    abortIfFail(connection && !connection->isConnected());
    SlotPrivate::attachToSignal(*connection, *this);
    m_slots.push_back(std::move(connection));
    return m_slots.back().get();
}

void Signal::disconnect(Slot& connection)
{
    abortIfFail(connection.getSignal() == this);

    SlotPrivate::detachFromSignal(connection);

    if (m_emitting)
    {
        auto predicate = [&connection](auto& item)
        {
            return &connection == item.get();
        };
        auto it = std::find_if(m_slots.begin(), m_slots.end(), predicate);
        it->reset();
    }
    else
    {
        // Compact disconnected connections.
        std::erase(m_slots, SlotPtr());
    }
}

std::size_t Signal::getConnectionCount() const
{
    if (m_emitting)
    {
        auto result = std::size_t(0u);
        for (auto& connection : m_slots)
        {
            if (connection->isConnected())
            {
                ++result;
            }
        }
        return result;
    }

    return m_slots.size();
}

}
