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

#include <gtest/gtest.h>
#include "utils/domain_test_environment.hpp"

#include <meta/meta.hpp>
#include <meta/metadata/factory.hpp>
#include <meta/metadata/metaclass.hpp>
#include <meta/metadata/metaobject.hpp>


namespace
{

class MetaObjectTests : public DomainTestEnvironment
{
protected:
    meta::MetaObjectPtr m_testObject;
    void SetUp() override
    {
        // Single threaded
        initializeDomain(false, true);
        m_testObject = meta::MetaObject::create("test object");
    }

    struct TestSlot
    {
        bool activated = false;
        void operator()()
        {
            activated = true;
        }
    };
};

}

TEST_F(MetaObjectTests, invokeMetaMethod)
{
    auto result = invoke(m_testObject, "getName");
    EXPECT_TRUE(result);
    EXPECT_EQ("test object", static_cast<std::string_view>(*result));
}

TEST_F(MetaObjectTests, invokeInexistentMetaMethod)
{
    auto result = invoke(m_testObject, "wombat");
    EXPECT_FALSE(result);
}

TEST_F(MetaObjectTests, listenOnDeleted)
{
    auto onDeleted = TestSlot();

    m_testObject->deleted.connect(onDeleted, &TestSlot::operator());
    m_testObject.reset();
    EXPECT_TRUE(onDeleted.activated);
}


class TestClass : public meta::MetaObject
{
public:
    using VoidSignalType = meta::SignalType<void()>;

    VoidSignalType test{"test"};

    META_CLASS("TestClass", TestClass, meta::MetaObject)
    {
    };

    static std::shared_ptr<TestClass> create(std::string_view name)
    {
        return std::shared_ptr<TestClass>(new TestClass(name));
    }

protected:
    explicit TestClass(std::string_view name) :
        meta::MetaObject(name)
    {
    }
};

TEST_F(MetaObjectTests, wrapSignalInCallable)
{
    // auto slot = TestSlot();
    // auto object = TestClass::create("test");
    // object->test.connect(slot, &TestSlot::operator());

    // // auto metaclass = static_cast<const TestClass::MetaClassType*>(TestClass::getStaticMetaClass());
    // // metaclass->testWrapper.apply(&object->test, meta::PackagedArguments());

    // EXPECT_TRUE(slot.activated);
}
