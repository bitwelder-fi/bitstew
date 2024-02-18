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

#include <gtest/gtest.h>

#include <meta/meta.hpp>
#include <meta/metadata/metaclass.hpp>
#include <meta/object.hpp>
#include <meta/object_extensions/object_extension.hpp>
#include <meta/library_config.hpp>

#include <meta/object_extensions/invokable.hpp>

#include "utils/domain_test_environment.hpp"

namespace
{

class ObjectTest : public DomainTestEnvironment
{
protected:
    void SetUp() override
    {
        DomainTestEnvironment::initializeDomain(true, true);
    }
    void TearDown() override
    {
        // Ensure tracer completes its job.
        auto tracer = meta::Library::instance().tracer();
        if (tracer->isBusy())
        {
            tracer->wait();
        }
        DomainTestEnvironment::TearDown();
    }
};

DECLARE_INVOKABLE(GetName, "getName", &meta::Object::getName);

}


TEST_F(ObjectTest, addExtension)
{
    auto object = meta::Object::create("test");
    auto getName = GetName::create();
    object->addExtension(getName);

    EXPECT_CALL(*m_mockPrinter, log("Extension 'getName' already extends the object."));
    object->addExtension(getName);
}

TEST_F(ObjectTest, findExtension)
{
    auto object = meta::Object::create("test");
    auto getName = GetName::create();
    object->addExtension(getName);

    EXPECT_EQ(getName, object->findExtension("getName"));
}

TEST_F(ObjectTest, removeExtension)
{
    auto object = meta::Object::create("test");
    auto getName = GetName::create();
    object->addExtension(getName);
    EXPECT_EQ(object, getName->getObject());

    object->removeExtension(*getName);
    EXPECT_EQ(nullptr, getName->getObject());
}

TEST_F(ObjectTest, objectInvoke_getName)
{
    auto object = meta::Object::create("test");
    auto getName = GetName::create();
    object->addExtension(getName);

    auto result = object->invoke("getName");
    ASSERT_NE(std::nullopt, result);
    EXPECT_EQ(std::string_view("test"), static_cast<std::string_view>(*result));
}

TEST_F(ObjectTest, metaInvoke_getName)
{
    auto object = meta::Object::create("test");
    auto getName = GetName::create();
    object->addExtension(getName);

    auto result = meta::invoke(object, "getName");
    ASSERT_NE(std::nullopt, result);
    EXPECT_EQ(std::string_view("test"), static_cast<std::string_view>(*result));
}
