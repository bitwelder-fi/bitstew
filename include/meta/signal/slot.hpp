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

#ifndef META_CONNECTION_HPP
#define META_CONNECTION_HPP

#include <meta/meta_api.hpp>
#include <utils/function_traits.hpp>
#include <meta/arguments/argument_type.hpp>
#include <meta/metadata/callable.hpp>

namespace meta
{

class Signal;
class Slot;
using SlotPtr = std::unique_ptr<Slot>;

/// %Slot represents a connection to a signal. A slot is a token which holds the signal connected,
/// and the function, method, metamethod, functor or lambda the signal is connected to.
class META_API Slot : protected Callable
{
    friend struct SlotPrivate;

public:
    /// Constructor. Creates a slot with an object and its method.
    /// \param object The object of the slot.
    /// \param function The method.
    template <class Class, class Function>
    explicit Slot(Class& object, Function function) :
        Callable("object_slot", function),
        m_object(static_cast<typename traits::function_traits<Function>::object*>(&object))
    {
        static_assert(std::is_member_function_pointer_v<Function> &&
                      std::is_base_of_v<typename traits::function_traits<Function>::object, Class>,
                      "Object differs from the function");
        if constexpr (traits::function_traits<Function>::arity > 0u)
        {
            m_passConnection = traits::is_same_arg<Function, Slot*, 0u>::value;
        }
    }

    /// Constructor. Creates a slot with a function, or a lambda.
    /// \param function The function of the slot.
    template <class Function>
    explicit Slot(Function function) :
        Callable("function_slot", std::move(function))
    {
        if constexpr (traits::function_traits<Function>::arity > 0u)
        {
            m_passConnection = traits::is_same_arg<Function, Slot*, 0u>::value;
        }
    }

    template <class Class, class Function>
    static SlotPtr create(Class& object, Function function)
    {
        return std::make_unique<Slot>(object, function);
    }
    template <class Function>
    static SlotPtr create(Function function)
    {
        return std::make_unique<Slot>(function);
    }

    /// Move.
    Slot(Slot&& other);
    Slot& operator=(Slot&& other);
    void swap(Slot& other);

    /// Returns the state of the slot.
    /// \return If the slot is connected, \e true, otherwise \e false.
    bool isConnected() const
    {
        return m_signal != nullptr;
    }

    /// The signal of the slot. A slot has a signal when it is connected to that signal.
    /// \return The signal to which the slot is connected, or \e nullptr if the slot is not connected.
    Signal* getSignal() const
    {
        return m_signal;
    }

    /// Disconnects the slot from the signal.
    /// \return If the slot was connected, returns \e true, otherwise \e false.
    bool disconnect();

    /// Activates the slot by calling the function or the method of the slot.
    /// \param args The arguments to pass to the slot.
    bool activate(const PackagedArguments& arguments);

private:
    /// The object of the slot, if the slot is a method of an object.
    ArgumentData m_object;
    /// The signal to which the slot is connected.
    Signal* m_signal = nullptr;
    /// If the slot's first argument is the connection object itself.
    bool m_passConnection = false;
};

}

#endif
