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

#include "utils/domain_test_environment.hpp"

#include <stew/meta/object.hpp>
#include <stew/meta/object_extensions/invokable.hpp>
#include <stew/meta/object_extensions/connection.hpp>
#include <stew/meta/object_extensions/signal.hpp>

#include <sstream>
#include <chrono>

namespace
{

void function()
{
    STEW_LOG_INFO(__FUNCTION__);
}

void function(int i)
{
    STEW_LOG_INFO(__FUNCTION__ << "(" << i << ")");
}

void selfDisconnect(stew::ObjectExtension* self)
{
    self->disconnectTarget();
}

using VoidSignal = stew::Signal<void()>;
using IntSignal = stew::Signal<void(int)>;

DECLARE_INVOKABLE_OVERLOAD(VoidSlot, "voidSlot", void(*)(), &function);
DECLARE_INVOKABLE_OVERLOAD(IntSlot, "intSlot", void(*)(int), &function);
DECLARE_INVOKABLE(SelfDisconnect, "selfDisconnect", &selfDisconnect);

class Object : public stew::Object
{
public:
    VoidSignal sigVoid{*this, "sigVoid"};
    IntSignal sigInt{*this, "sigInt"};

    void connectInSlot()
    {
        auto innerSlot = ConnectInSlot::create(generateName());
        addExtension(innerSlot);
        sigVoid.connect(innerSlot);
    }

    DECLARE_INVOKABLE(ConnectInSlot, "connectInSlot", &Object::connectInSlot);

    STEW_CLASS("Object", Object, stew::Object)
    {
        STEW_EXTENSION(VoidSignal);
        STEW_EXTENSION(IntSignal);
        STEW_EXTENSION(ConnectInSlot);
    };

    static auto create(std::string_view name)
    {
        auto object = std::shared_ptr<Object>(new Object(name));
        object->initialize();
        return object;
    }

protected:
    explicit Object(std::string_view name) :
        stew::Object(name)
    {
    }

    static std::string generateName()
    {
        std::stringstream ss;
        ss << "slot_" << std::chrono::steady_clock().now().time_since_epoch().count();
        return ss.str();
    }
};


class SignalTests : public DomainTestEnvironment
{
protected:
    void SetUp() override
    {
        DomainTestEnvironment::initializeDomain(true, true);
    }
};

using GenericSignalTests = SignalTests;

}

TEST_F(GenericSignalTests, signalMetaName)
{
    EXPECT_EQ("void(*)()", VoidSignal::getStaticMetaClass()->getName());
    EXPECT_EQ("void(*)(int)", IntSignal::getStaticMetaClass()->getName());
}

TEST_F(GenericSignalTests, signalsOfObject)
{
    auto object = Object::create("test");
    EXPECT_NE(nullptr, object->findExtension("sigVoid"));
    EXPECT_NE(nullptr, object->findExtension("sigInt"));
}

TEST_F(GenericSignalTests, connect)
{
    VoidSignal signal;
    auto slot = VoidSlot::create();
    auto connection = signal.connect(slot);
    EXPECT_TRUE(connection->isValid());
    EXPECT_EQ(static_cast<stew::ObjectExtensionPtr>(signal), connection->getSource());
    EXPECT_EQ(slot, connection->getTarget());

    auto slot2 = IntSlot::create();
    connection = signal.connect(slot2);
    EXPECT_TRUE(connection->isValid());

    EXPECT_EQ(2u, signal.getConnectionCount());
}

TEST_F(GenericSignalTests, disconnect)
{
    VoidSignal signal;
    auto slot = VoidSlot::create();
    auto connection = signal.connect(slot);

    signal.disconnect(*connection);
    EXPECT_FALSE(connection->isValid());
}

TEST_F(GenericSignalTests, connectToSignal)
{
    IntSignal signal("sigInt");
    VoidSignal signal2("sigVoid");

    auto connection = signal.connect(signal2);
    EXPECT_TRUE(connection->isValid());
}

TEST_F(GenericSignalTests, connectByName)
{
    auto object = Object::getStaticMetaClass()->create<Object>("test");
    auto connection = object->sigVoid.connect("connectInSlot");
    ASSERT_NE(nullptr, connection);
}


TEST_F(GenericSignalTests, trigger)
{
    VoidSignal signal;
    EXPECT_EQ(0u, signal.trigger());
}

TEST_F(GenericSignalTests, invokeSignal)
{
    auto object = Object::create("test");
    auto result = object->invoke("sigVoid");
    ASSERT_TRUE(result);
    EXPECT_EQ(0, static_cast<int>(*result));
}

TEST_F(GenericSignalTests, invokeSigVoidWithArguments)
{
    auto object = Object::create("test");
    auto result = object->invoke("sigVoid", 1);
    ASSERT_TRUE(result);
    EXPECT_EQ(0, static_cast<int>(*result));
}

TEST_F(GenericSignalTests, invokeSigIntWithArguments)
{
    auto object = Object::create("test");
    auto result = object->invoke("sigInt", 1);
    ASSERT_TRUE(result);
    EXPECT_EQ(0, static_cast<int>(*result));
}

TEST_F(GenericSignalTests, triggerSigVoidWithConnection)
{
    VoidSignal signal;
    auto slot = VoidSlot::create();
    signal.connect(slot);

    EXPECT_CALL(*m_mockPrinter, log("function"));
    EXPECT_EQ(1u, signal.trigger());
}

TEST_F(GenericSignalTests, triggerSigIntWithConnection)
{
    IntSignal signal;
    auto voidSlot = VoidSlot::create();
    auto intSlot = IntSlot::create();
    signal.connect(voidSlot);
    signal.connect(intSlot);

    EXPECT_CALL(*m_mockPrinter, log("function"));
    EXPECT_CALL(*m_mockPrinter, log("function(10)"));
    EXPECT_EQ(2u, signal.trigger(10));
}

TEST_F(GenericSignalTests, triggerSigIntWithSigVoidAsConnection)
{
    IntSignal sigInt;
    VoidSignal sigVoid;
    auto slot = VoidSlot::create();
    sigInt.connect(sigVoid);
    sigVoid.connect(slot);

    EXPECT_CALL(*m_mockPrinter, log("function"));
    EXPECT_EQ(1, sigInt.trigger(10));
}

TEST_F(GenericSignalTests, triggerSigIntConnectedToSigVoidFails)
{
    IntSignal sigInt;
    VoidSignal sigVoid;

    sigVoid.connect(sigInt);
    EXPECT_EQ(0, sigVoid.trigger());
}

TEST_F(GenericSignalTests, triggerConnectionToLambda)
{
    VoidSignal signal;
    auto lambda = []()
    {
        STEW_LOG_INFO("lambda");
    };
    using Lambda = stew::Invokable<decltype(lambda), lambda>;
    auto invokable = Lambda::create("slot");

    auto connection = signal.connect(invokable);
    ASSERT_TRUE(connection->isValid());

    EXPECT_CALL(*m_mockPrinter, log("lambda"));
    EXPECT_EQ(1, signal.trigger());
}

TEST_F(GenericSignalTests, disconnectInSlot)
{
    VoidSignal signal;
    auto slot = SelfDisconnect::create();
    auto connection = signal.connect(slot);

    EXPECT_EQ(1, signal.trigger());
    EXPECT_FALSE(connection->isValid());
}

TEST_F(GenericSignalTests, connectInSlot)
{
    auto object = Object::getStaticMetaClass()->create<Object>("test");
    object->sigVoid.connect(object->findExtension("connectInSlot"));

    // Each trigger doubles the connection.
    EXPECT_EQ(1, object->sigVoid.trigger());
    EXPECT_EQ(2, object->sigVoid.trigger());
    EXPECT_EQ(4, object->sigVoid.trigger());
    EXPECT_EQ(8, object->sigVoid.trigger());
}
