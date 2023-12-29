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

#ifndef META_SIGNAL_HPP
#define META_SIGNAL_HPP

#include <meta/meta_api.hpp>
#include <meta/arguments/argument_type.hpp>
#include <meta/metadata/callable.hpp>
#include <meta/signal/slot.hpp>

#include <deque>

namespace meta
{

/// The connection type.
class Slot;
using SlotPtr = std::unique_ptr<Slot>;

/// %Signal defines the core of meta signals. Signals are the observants of the observer pattern.
/// \see SignalType
class META_API Signal
{
    DISABLE_COPY(Signal);
    DISABLE_MOVE(Signal);

public:
    /// Checks whether the signal is valid.
    /// \return If the signal is valid, returns \e true, otherwise \e false.
    bool isValid() const;

    /// Activates the signal with the packaged arguments.
    /// \param arguments The packaged arguments to pass as signal arguments.
    /// \return The number of activated connections. If the value is 0, there was no connection
    ///         activated. If the value is -1, the arguments do not match the signature of the signal.
    int emit(const PackagedArguments& arguments = PackagedArguments());

    /// Adds a slot to the signal.
    /// \param slot The slot to add to the signal.
    Slot* connect(SlotPtr slot);

    /// Removes a slot from the signal.
    /// \param connection The connection to remove from the signal.
    void disconnect(Slot& connection);

    /// Returns the number of valid connections.
    /// \return The number of connections which are valid.
    std::size_t getConnectionCount() const;

    /// Connects a method of a receiver to the signal.
    /// \tparam Function The function type.
    /// \param receiver The receiver object.
    /// \param method The method to connect to the signal.
    /// \return If the connection suceeds, returns the slot, otherwise returns \e nullptr.
    template <typename Function>
    Slot* connect(typename traits::function_traits<Function>::object& receiver, Function method)
    {
        auto slot = std::make_unique<Slot>(receiver, method);
        return connect(std::move(slot));
    }

    /// Connects a function, or a lambda to the signal.
    /// \tparam Function The function type.
    /// \param f The function to connect to the signal.
    /// \return If the connection suceeds, returns the slot, otherwise returns \e nullptr.
    template <typename Function>
    typename std::enable_if<!std::is_base_of_v<meta::Signal, Function>, Slot*>::type
    connect(Function f)
    {
        auto slot = std::make_unique<Slot>(std::move(f));
        return connect(std::move(slot));
    }

protected:
    /// Constructor.
    explicit Signal(std::string_view name);
    /// Destructor.
    virtual ~Signal() = default;

    /// Activates the slots of the signal.
    /// \param arguments The packaged arguments to pass as signal arguments.
    /// \return The number of activated slots.
    int activateSlots(const PackagedArguments& arguments = PackagedArguments());

    /// Verifies the packaged arguments against the signature of the signal.
    /// \param arguments The packaged arguments to verify against the signature.
    /// \return If the packaged arguments match the signature of the signal, returns \e true, otherwise
    ///         \e false.
    virtual bool verifySignature(const PackagedArguments& arguments) const = 0;

    /// The container type of the connected slots.
    using SlotContainer = std::deque<SlotPtr>;
    /// The container of the connected slots.
    SlotContainer m_slots;
    /// The name of the signal.
    const std::string m_name;
    /// Whether the signal is emitting.
    bool m_emitting = false;
};


/// SignalType template specialization for a generic signature.
template <class Signature>
class SignalType;

/// SignalType template, specialization with a signature of void function with arbitrary arguments. You
/// can connect methods, metamethods, functions, functors or lambdas to a signal using the relevant
/// connect() methods, or create a slot object with the callables, and connect the slot to the signal.
/// To disconnect a slot, call disconnect() on the signal, or call disconnect() on the slot.
///
/// A callable can have less arguments than the signal, however the arguments of the callable at a
/// given position must be identical with the arguments of the signal at the same position. You can
/// connect a callable with no arguments to a signal with no, or any argument.
///
/// A signal can also act as a slot. The same rules on the arguments apply also to the signals.
template <class... Arguments>
class META_TEMPLATE_API SignalType<void(Arguments...)> : public Signal
{
public:
    /// The signature of the signal.
    using Signature = void(*)(Arguments...);

    /// Constructor. Creates a non-name signal.
    explicit SignalType() :
        SignalType("noname")
    {
    }

    /// Constructor. Creates a signal with a name given as argument.
    explicit SignalType(std::string_view name) :
        Signal(name)
    {
    }

    /// The emit operator, with arguments specified in the signature of the signal.
    /// \param arguments The arguments of the signal.
    /// \return The number of slots activated.
    int operator()(Arguments&&... arguments)
    {
        auto pack = PackagedArguments(std::forward<Arguments>(arguments)...);
        return activateSlots(pack);
    }

protected:
    /// Overrides Signal::verifySignature().
    bool verifySignature(const PackagedArguments& arguments) const override
    {
        constexpr auto arity = sizeof...(Arguments);
        if (arguments.getSize() < arity)
        {
            return false;
        }

        try
        {
            auto tupleArgs = arguments.toTuple<Signature>();
            return std::is_same_v<typename traits::function_traits<Signature>::arg_types, decltype(tupleArgs)>;
        }
        catch (...)
        {
            return false;
        }
    }
};

}

#endif
