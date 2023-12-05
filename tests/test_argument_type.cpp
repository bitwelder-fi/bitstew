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

#include "utils/domain_test_environment.hpp"

#include <meta/arguments/argument_type.hpp>
#include <meta/meta.hpp>

namespace
{

class ArgumentTestBase : public DomainTestEnvironment
{
protected:
    void SetUp() override
    {
        // Single threaded
        initializeDomain(false, true);
    }
};

using PackagedArguments = ArgumentTestBase;

void testFunction3(std::string a1, int a2, float a3)
{
    META_LOG_INFO(a1 << ", " << a2 << ", " << a3 );
}

void testFunction2(std::string a1, int a2)
{
    META_LOG_INFO(a1 << ", " << a2);
}

auto lambda3 = [](std::string a1, int a2, float a3)
{
    META_LOG_INFO(a1 << ", " << a2 << ", " << a3 );
};

auto lambda2 = [](std::string a1, int a2)
{
    META_LOG_INFO(a1 << ", " << a2);
};

using Functor3 = std::function<void(std::string a1, int a2, float a3)>;
using Functor2 = std::function<void(std::string a1, int a2)>;

struct Class
{
    void method(std::string a1, int a2, float a3)
    {
        META_LOG_INFO(a1 << ", " << a2 << ", " << a3 );
    }
    void method2(std::string a1, int a2)
    {
        META_LOG_INFO(a1 << ", " << a2);
    }
};

}

TEST(ArgumentData, invalidArgumentData)
{
    auto voidArgument = meta::ArgumentData();
    EXPECT_FALSE(voidArgument.has_value());
}

TEST(ArgumentData, testArgumentData)
{
    auto argument = meta::ArgumentData(std::string("one"));
    EXPECT_EQ(std::string("one"), argument);
}

TEST_F(PackagedArguments, testPackArguments)
{
    auto pack = meta::PackagedArguments(std::string("one"), 2, 3.3f);
    ASSERT_EQ(3u, pack.getSize());
    EXPECT_EQ("one", pack.get<std::string>(0u));
    EXPECT_EQ(2, pack.get<int>(1u));
    EXPECT_EQ(3.3f, pack.get<float>(2u));
}

TEST_F(PackagedArguments, unpackUsingLambdaSignature)
{
    auto pack = meta::PackagedArguments(std::string("one"), 2, 3.3f);
    auto tuplePack = pack.toTuple(lambda3);

    EXPECT_CALL(*m_mockPrinter, log("one, 2, 3.3"));
    std::apply(lambda3, tuplePack);
}

TEST_F(PackagedArguments, unpackUsingLambdaSignatureWithLesserArguments)
{
    auto pack = meta::PackagedArguments(std::string("one"), 2, 3.3f);
    auto tuplePack = pack.toTuple(lambda2);

    EXPECT_CALL(*m_mockPrinter, log("one, 2"));
    std::apply(lambda2, tuplePack);
}

TEST_F(PackagedArguments, unpackUsingFunctorSignature)
{
    Functor3 functor = lambda3;
    auto pack = meta::PackagedArguments(std::string("one"), 2, 3.3f);
    auto tuplePack = pack.toTuple(functor);

    EXPECT_CALL(*m_mockPrinter, log("one, 2, 3.3"));
    std::apply(functor, tuplePack);
}

TEST_F(PackagedArguments, unpackUsingFunctorSignatureWithLesserArguments)
{
    Functor2 functor = lambda2;
    auto pack = meta::PackagedArguments(std::string("one"), 2, 3.3f);
    auto tuplePack = pack.toTuple(functor);

    EXPECT_CALL(*m_mockPrinter, log("one, 2"));
    std::apply(functor, tuplePack);
}

TEST_F(PackagedArguments, unpackUsingFunctionSignature)
{
    auto pack = meta::PackagedArguments(std::string("one"), 2, 3.3f);
    auto tuplePack = pack.toTuple(testFunction3);

    EXPECT_CALL(*m_mockPrinter, log("one, 2, 3.3"));
    std::apply(testFunction3, tuplePack);
}

TEST_F(PackagedArguments, unpackUsingFunctionWithLesserArguments)
{
    auto pack = meta::PackagedArguments(std::string("one"), 2, 3.3f);
    auto tuplePack = pack.toTuple(testFunction2);

    EXPECT_CALL(*m_mockPrinter, log("one, 2"));
    std::apply(testFunction2, tuplePack);
}

TEST_F(PackagedArguments, unpackUsingMethodSignature)
{
    auto object = Class();
    auto pack = meta::PackagedArguments(std::string("one"), 2, 3.3f);
    auto tuplePack = std::tuple_cat(std::make_tuple(&object), pack.toTuple(&Class::method));

    EXPECT_CALL(*m_mockPrinter, log("one, 2, 3.3"));
    std::apply(&Class::method, tuplePack);
}

TEST_F(PackagedArguments, unpackUsingMethodSignatureWithLesserArguments)
{
    auto object = Class();
    auto pack = meta::PackagedArguments(std::string("one"), 2, 3.3f);
    auto tuplePack = std::tuple_cat(std::make_tuple(&object), pack.toTuple(&Class::method2));

    EXPECT_CALL(*m_mockPrinter, log("one, 2"));
    std::apply(&Class::method2, tuplePack);
}


TEST_F(PackagedArguments, invokeLambda)
{
    auto pack = meta::PackagedArguments(std::string("one"), 2, 3.3f);

    EXPECT_CALL(*m_mockPrinter, log("one, 2, 3.3"));
    meta::invoke(lambda3, pack);
}

TEST_F(PackagedArguments, invokeLambdaWithLesserArguments)
{
    auto pack = meta::PackagedArguments(std::string("one"), 2, 3.3f);

    EXPECT_CALL(*m_mockPrinter, log("one, 2"));
    meta::invoke(lambda2, pack);
}

TEST_F(PackagedArguments, invokeFunctor)
{
    Functor3 functor = lambda3;
    auto pack = meta::PackagedArguments(std::string("one"), 2, 3.3f);

    EXPECT_CALL(*m_mockPrinter, log("one, 2, 3.3"));
    meta::invoke(functor, pack);
}

TEST_F(PackagedArguments, invokeFunctorWithLesserArguments)
{
    Functor2 functor = lambda2;
    auto pack = meta::PackagedArguments(std::string("one"), 2, 3.3f);

    EXPECT_CALL(*m_mockPrinter, log("one, 2"));
    meta::invoke(functor, pack);
}

TEST_F(PackagedArguments, invokeFunction)
{
    auto pack = meta::PackagedArguments(std::string("one"), 2, 3.3f);

    EXPECT_CALL(*m_mockPrinter, log("one, 2, 3.3"));
    meta::invoke(testFunction3, pack);
}

TEST_F(PackagedArguments, invokeFunctionWithLesserArguments)
{
    auto pack = meta::PackagedArguments(std::string("one"), 2, 3.3f);

    EXPECT_CALL(*m_mockPrinter, log("one, 2"));
    meta::invoke(testFunction2, pack);
}

TEST_F(PackagedArguments, invokeMethod)
{
    auto object = Class();
    auto pack = meta::PackagedArguments(std::string("one"), 2, 3.3f);

    EXPECT_CALL(*m_mockPrinter, log("one, 2, 3.3"));
    meta::invoke(&Class::method, &object, pack);
}

TEST_F(PackagedArguments, invokeMethodWithLesserArguments)
{
    auto object = Class();
    auto pack = meta::PackagedArguments(std::string("one"), 2, 3.3f);

    EXPECT_CALL(*m_mockPrinter, log("one, 2"));
    meta::invoke(&Class::method2, &object, pack);
}
