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
};

}


TEST_F(ObjectTest, invoke_getName)
{
    auto metaClass = meta::Object::getStaticMetaClass();
    auto object = metaClass->create<meta::Object>("object");
    ASSERT_NE(nullptr, object);

    using Invokable = meta::Invokable<decltype(&meta::Object::getName), &meta::Object::getName>;
    auto metaGetName = Invokable::create("getName");
    object->addExtension(metaGetName);
    auto result = meta::invoke(object, "getName");
    ASSERT_NE(std::nullopt, result);
    EXPECT_EQ(std::string_view("object"), static_cast<std::string_view>(*result));
}
