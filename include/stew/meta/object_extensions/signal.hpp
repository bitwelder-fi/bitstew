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

#ifndef STEW_SIGNAL_HPP
#define STEW_SIGNAL_HPP

#include <stew/dynamic_type/packaged_arguments.hpp>
#include <stew/stew_api.hpp>
#include <stew/forwards.hpp>
#include <stew/meta/object.hpp>
#include <stew/meta/object_extensions/object_extension.hpp>

#include <string_view>

namespace stew
{

/// The %SignalExtension defines the core functionality of a meta-signal. It holds the slots connected,
/// the activation and the blocked state of a signal. Use the SignalType<> template to declare a signal
/// in your application.
///
/// A generic signal is activated using the trigger() method. When a signal is activated, its connected
/// slots get invoked. Connections created within an activated slot is left out from the current signal
/// activation.
class STEW_API SignalExtension : public ObjectExtension
{
public:
    STEW_CLASS("SignalExtension", SignalExtension, ObjectExtension)
    {
    };

    /// Destructor.
    ~SignalExtension();

    /// Returns whether the signal is processing its connections.
    inline bool isTriggering() const
    {
        return m_connections.getRefCount() > 0u;
    }

    /// Connects an object extension to the signal, and returns the connection token.
    /// \param slot The object extension as the slot of the connection.
    /// \return The connection token.
    ConnectionPtr connect(ObjectExtensionPtr slot);

    /// Connects an object extension identified by the extensionName to the signal, and returns the
    /// connection token. The method is only applicable when the signal is attached to an object.
    /// \param extensionName The name of the object extension as the slot of the connection.
    /// \return The connection token.
    ConnectionPtr connect(std::string_view extensionName);

    /// Disconnects a connection. The signal of the connection token must be the signal itself.
    /// \param connection The connection to disconnect. On return, the connection is reset.
    void disconnect(Connection& connection);

    /// Tries to reset the signal extension, disconnecting the connections. The method fails if
    /// the reset is called during signal activation.
    /// \return On siuccessful reset returns \e true, otherwise \e false.
    bool tryDisconnect();

    /// Returns the number of valid connections.
    /// \return The number of connections which are valid.
    std::size_t getConnectionCount() const;

    /// Triggers the signal extension with the passed arguments. Returns the number of slots activated.
    /// \param arguments The arguments of the signal.
    /// \return The number of slots activated, or
    ///         -# -1 if the signal activation failed.
    ///         -# 0 if the signal is already active, or there was no connection activated.
    template <typename... Arguments>
    int trigger(Arguments... args)
    {
        auto result = run(PackagedArguments(args...));
        return result != std::nullopt ? static_cast<int>(*result) : -1;
    }

protected:
    /// Constructs the signal extension
    explicit SignalExtension(std::string_view name);

    /// Overrides ObjectExtension::runOverride() to activate the slots of the signal.
    /// \param arguments The packaged arguments to pass as signal arguments.
    /// \return The number of activated slots.
    ReturnValue runOverride(PackagedArguments arguments) final;

    /// Verifies the packaged arguments against the signature of the signal.
    /// \param arguments The packaged arguments to verify against the signature.
    /// \return If the packaged arguments match the signature of the signal, returns \e true, otherwise
    ///         \e false.
    virtual bool verifySignature(PackagedArguments arguments) const = 0;
};

template <class... Arguments>
class STEW_TEMPLATE_API SignalExtensionType : public SignalExtension
{
    using SelfType = SignalExtensionType<Arguments...>;
public:
    using Signature = void(*)(Arguments...);

    AUTO_STEW_CLASS(Signature, SelfType, SignalExtension)
    {
    };
    static auto create(std::string_view name)
    {
        return std::shared_ptr<SelfType>(new SignalExtensionType(name));
    }

protected:
    explicit SignalExtensionType(std::string_view name) :
        SignalExtension(name)
    {
    }

    /// Overrides SignalExtension::verifySignature().
    bool verifySignature(PackagedArguments arguments) const final
    {
        constexpr auto arity = sizeof...(Arguments);
        if (arguments.getSize() < arity)
        {
            return false;
        }

        try
        {
            auto tupleArgs = arguments.toTuple<Signature>();
            return std::is_same_v<typename function_traits<Signature>::arg::types, decltype(tupleArgs)>;
        }
        catch (...)
        {
            return false;
        }
    }
};


template <class Signature>
class Signal;

/// The Signal template defines the signature of a meta-signal. Use the template to define the type
/// of the signal. You can declare multiple signals with the same type. The name of the signal is
/// defined upon instantiation.
///
/// The %Signal wraps a SignalExtension, and verifies the packaged argument against the signature of
/// the signal.
///
/// A callable slot may have less arguments than the signal, however the order of the slot arguments
/// must be identical with the argument order of the signal. You can connect a callable slot with no
/// arguments to a signal with any argument.
///
/// A signal can also act as a slot. The same rules on the arguments apply also to the signals.
template <class... Arguments>
class STEW_TEMPLATE_API Signal<void(Arguments...)>
{
    using ExtensionType = SignalExtensionType<Arguments...>;

public:
    /// The signature of the signal.
    using Signature = void(*)(Arguments...);

    /// Constructor. Creates a no-name signal.
    explicit Signal() :
        Signal("__noname__")
    {
    }

    /// Constructor. Creates a signal with a name given as argument.
    explicit Signal(std::string_view name) :
        m_extension(ExtensionType::create(name))
    {
    }

    /// Constructor. Creates a signal for an Object with a name given as argument.
    explicit Signal(Object& host, std::string_view name) :
        Signal(name)
    {
        host.addExtension(m_extension);
    }

    /// Destructor.
    ~Signal()
    {
        m_extension->tryDisconnect();
        m_extension.reset();
    }

    /// Connects an object extension to the signal, and returns the connection token.
    /// \param slot The object extension as the slot of the connection.
    /// \return The connection token.
    inline ConnectionPtr connect(ObjectExtensionPtr slot)
    {
        return m_extension->connect(slot);
    }

    /// Connects an object extension identified by the extensionName to the signal, and returns the
    /// connection token. The method is only applicable when the signal is attached to an object.
    /// \param extensionName The name of the object extension as the slot of the connection.
    /// \return The connection token.
    ConnectionPtr connect(std::string_view extensionName)
    {
        return m_extension->connect(extensionName);
    }

    /// Disconnects a connection. The signal of the connection token must be the signal itself.
    /// \param connection The connection to disconnect. On return, the connection is reset.
    inline void disconnect(Connection& slot)
    {
        m_extension->disconnect(slot);
    }

    /// Disconnect all signal connections.
    void disconnect()
    {
        std::static_pointer_cast<ObjectExtension>(m_extension)->disconnect();
    }

    /// Returns the number of valid connections.
    /// \return The number of connections which are valid.
    auto getConnectionCount() const
    {
        return m_extension->getConnectionCount();
    }

    /// Activates the signal with the packaged arguments.
    /// \param arguments The arguments of the signal.
    /// \return The number of slots activated, or
    ///         -# -1 if the signal activation failed.
    ///         -# 0 if the signal is already active, or there was no connection executed.
    int trigger(Arguments... arguments)
    {
        return m_extension->template trigger(arguments...);
    }

    /// Returns the static metaclass of the signal extension of the type.
    static const MetaClass* getStaticMetaClass()
    {
        return ExtensionType::getStaticMetaClass();
    }

    /// Returns the name of the signal.
    inline std::string_view getName() const
    {
        return m_extension->getName();
    }

    /// Cast operator to use a signal as a slot.
    operator ObjectExtensionPtr() const
    {
        return m_extension;
    }

protected:
    /// The object extension of the signal.
    SignalExtensionPtr m_extension;
};

}

#endif
