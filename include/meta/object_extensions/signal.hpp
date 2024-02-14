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

#ifndef META_SIGNAL_HPP
#define META_SIGNAL_HPP

#include <meta/arguments/packaged_arguments.hpp>
#include <meta/meta_api.hpp>
#include <meta/forwards.hpp>
#include <meta/object.hpp>
#include <meta/object_extensions/object_extension.hpp>

#include <string_view>

namespace meta
{

/// The %SignalExtension defines the core functionality of a meta signal. It holds the slots connected,
/// the activation and the blocked state of a signal. Use the SignalType<> template to declare a signal
/// in your application.
///
/// A generic signal is activated using the trigger() method. When a signal is activated, its connected
/// slots get invoked. Connections created within an activated slot is left out from the current signal
/// activation.
class META_API SignalExtension : public ObjectExtension
{
public:
    META_CLASS("SignalExtension", SignalExtension, ObjectExtension)
    {
    };

    /// Returns whether the signal is processing its connections.
    inline bool isTriggering() const
    {
        return m_triggering;
    }

    /// Connects an object extension to the signal, and returns the connection token.
    /// \param slot The object extension as the slot of the connection.
    /// \return The connection token.
    Connection connect(ObjectExtensionPtr slot);

    /// Disconnects a connection. The signal of the connection token must be the signal itself.
    /// \param connection The connection to disconnect. On return, the connection is reset.
    void disconnect(Connection& connection);

    /// Tries to reset the signal extension, disconnecting the connections. The method fails if
    /// the reset is called during signal activation.
    /// \return On siuccessful reset returns \e true, otherwise \e false.
    bool tryReset();

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
        PackagedArguments pack{args...};
        auto result = run(pack);
        return result != std::nullopt ? static_cast<int>(*result) : -1;
    }

protected:
    /// Constructs the signal extension
    explicit SignalExtension(std::string_view name);

    /// Overrides ObjectExtension::runOverride() to activate the slots of the signal.
    /// \param arguments The packaged arguments to pass as signal arguments.
    /// \return The number of activated slots.
    ReturnValue runOverride(const PackagedArguments& arguments) final;

    /// Verifies the packaged arguments against the signature of the signal.
    /// \param arguments The packaged arguments to verify against the signature.
    /// \return If the packaged arguments match the signature of the signal, returns \e true, otherwise
    ///         \e false.
    virtual bool verifySignature(const PackagedArguments& arguments) const = 0;

    /// Whether the signal is activating its connections.
    bool m_triggering = false;
};

template <class... Arguments>
class META_TEMPLATE_API SignalExtensionType : public SignalExtension
{
    using SelfType = SignalExtensionType<Arguments...>;
public:
    using Signature = void(*)(Arguments...);

    AUTO_META_CLASS(Signature, SelfType, SignalExtension)
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
    bool verifySignature(const PackagedArguments& arguments) const final
    {
        constexpr auto arity = sizeof...(Arguments);
        if (arguments.getSize() < arity)
        {
            return false;
        }

        try
        {
            auto tupleArgs = arguments.toTuple<Signature>();
            return std::is_same_v<typename traits::function_traits<Signature>::arg::types, decltype(tupleArgs)>;
        }
        catch (...)
        {
            return false;
        }
    }
};


template <class Signature>
class Signal;

/// The Signal template defines the signature of a meta signal. Use the template to define the type
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
class META_TEMPLATE_API Signal<void(Arguments...)>
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
        if (m_extension->getObject())
        {
            m_extension->getObject()->removeExtension(*m_extension);
        }
    }

    /// Connects an object extension to the signal, and returns the connection token.
    /// \param slot The object extension as the slot of the connection.
    /// \return The connection token.
    inline Connection connect(ObjectExtensionPtr slot)
    {
        return m_extension->connect(slot);
    }

    /// Disconnects a connection. The signal of the connection token must be the signal itself.
    /// \param connection The connection to disconnect. On return, the connection is reset.
    inline void disconnect(Connection& slot)
    {
        m_extension->disconnect(slot);
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
