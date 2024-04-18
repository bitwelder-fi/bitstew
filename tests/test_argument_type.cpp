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

#include <stew/variable/packaged_arguments.hpp>
#include <stew/stew.hpp>

namespace
{

template <typename Function>
auto invokeFunction(Function function, const stew::PackagedArguments& arguments)
{
    if constexpr (std::is_member_function_pointer_v<Function>)
    {
        auto pack = arguments.toTuple<Function>();
        return std::apply(function, pack);
    }
    else
    {
        auto pack = arguments.toTuple<Function>();
        return std::apply(function, pack);
    }
}

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
    STEW_LOG_INFO(a1 << ", " << a2 << ", " << a3 );
}

void testFunction2(std::string a1, int a2)
{
    STEW_LOG_INFO(a1 << ", " << a2);
}

auto lambda3 = [](std::string a1, int a2, float a3)
{
    STEW_LOG_INFO(a1 << ", " << a2 << ", " << a3 );
};

auto lambda2 = [](std::string a1, int a2)
{
    STEW_LOG_INFO(a1 << ", " << a2);
};

using Functor3 = std::function<void(std::string a1, int a2, float a3)>;
using Functor2 = std::function<void(std::string a1, int a2)>;

struct Class
{
    void method(std::string a1, int a2, float a3)
    {
        STEW_LOG_INFO(a1 << ", " << a2 << ", " << a3 );
    }
    void method2(std::string a1, int a2)
    {
        STEW_LOG_INFO(a1 << ", " << a2);
    }
};

}

TEST(Argument, invalidArgumentData)
{
    auto voidArgument = stew::Argument();
    EXPECT_FALSE(voidArgument.has_value());
}

TEST(Argument, testArgumentData)
{
    auto argument = stew::Argument(std::string("one"));
    EXPECT_EQ(std::string("one"), static_cast<std::string>(argument));
}

TEST_F(PackagedArguments, buildArgumentData)
{
    auto arguments = stew::PackagedArguments();
    EXPECT_EQ(0u, arguments.getSize());
    arguments += std::string("one");
    EXPECT_EQ(1u, arguments.getSize());
    arguments += 2;
    EXPECT_EQ(2u, arguments.getSize());
}

TEST_F(PackagedArguments, testPackArguments)
{
    auto pack = stew::PackagedArguments(std::string("one"), 2, 3.3f);
    ASSERT_EQ(3u, pack.getSize());
    EXPECT_EQ("one", pack.get<std::string>(0u));
    EXPECT_EQ(2, pack.get<int>(1u));
    EXPECT_EQ(3.3f, pack.get<float>(2u));
}

TEST_F(PackagedArguments, deepCopy)
{
    auto pack = stew::PackagedArguments(std::string("one"), 2, 3.3f);
    auto pack2 = pack;

    EXPECT_EQ(pack, pack2);
    pack2 += 7;
    EXPECT_NE(pack, pack2);
}

TEST_F(PackagedArguments, unpackUsingLambdaSignature)
{
    auto pack = stew::PackagedArguments(std::string("one"), 2, 3.3f);
    auto tuplePack = pack.toTuple<decltype(lambda3)>();

    EXPECT_CALL(*m_mockPrinter, log("one, 2, 3.3"));
    std::apply(lambda3, tuplePack);
}

TEST_F(PackagedArguments, unpackUsingLambdaSignatureWithLesserArguments)
{
    auto pack = stew::PackagedArguments(std::string("one"), 2, 3.3f);
    auto tuplePack = pack.toTuple<decltype(lambda2)>();

    EXPECT_CALL(*m_mockPrinter, log("one, 2"));
    std::apply(lambda2, tuplePack);
}

TEST_F(PackagedArguments, unpackUsingFunctorSignature)
{
    Functor3 functor = lambda3;
    auto pack = stew::PackagedArguments(std::string("one"), 2, 3.3f);
    auto tuplePack = pack.toTuple<decltype(functor)>();

    EXPECT_CALL(*m_mockPrinter, log("one, 2, 3.3"));
    std::apply(functor, tuplePack);
}

TEST_F(PackagedArguments, unpackUsingFunctorSignatureWithLesserArguments)
{
    Functor2 functor = lambda2;
    auto pack = stew::PackagedArguments(std::string("one"), 2, 3.3f);
    auto tuplePack = pack.toTuple<decltype(functor)>();

    EXPECT_CALL(*m_mockPrinter, log("one, 2"));
    std::apply(functor, tuplePack);
}

TEST_F(PackagedArguments, unpackUsingFunctionSignature)
{
    auto pack = stew::PackagedArguments(std::string("one"), 2, 3.3f);
    auto tuplePack = pack.toTuple<decltype(&testFunction3)>();

    EXPECT_CALL(*m_mockPrinter, log("one, 2, 3.3"));
    std::apply(testFunction3, tuplePack);
}

TEST_F(PackagedArguments, unpackUsingFunctionWithLesserArguments)
{
    auto pack = stew::PackagedArguments(std::string("one"), 2, 3.3f);
    auto tuplePack = pack.toTuple<decltype(&testFunction2)>();

    EXPECT_CALL(*m_mockPrinter, log("one, 2"));
    std::apply(testFunction2, tuplePack);
}

TEST_F(PackagedArguments, unpackUsingMethodSignature)
{
    auto object = Class();
    auto pack = stew::PackagedArguments(&object, std::string("one"), 2, 3.3f);
    auto tuplePack = pack.toTuple<decltype(&Class::method)>();

    EXPECT_CALL(*m_mockPrinter, log("one, 2, 3.3"));
    std::apply(&Class::method, tuplePack);
}

TEST_F(PackagedArguments, unpackUsingMethodSignatureWithLesserArguments)
{
    auto object = Class();
    auto pack = stew::PackagedArguments(&object, std::string("one"), 2, 3.3f);
    auto tuplePack = pack.toTuple<decltype(&Class::method2)>();

    EXPECT_CALL(*m_mockPrinter, log("one, 2"));
    std::apply(&Class::method2, tuplePack);
}


TEST_F(PackagedArguments, invokeLambda)
{
    auto pack = stew::PackagedArguments(std::string("one"), 2, 3.3f);

    EXPECT_CALL(*m_mockPrinter, log("one, 2, 3.3"));
    invokeFunction(lambda3, pack);
}

TEST_F(PackagedArguments, invokeLambdaWithLesserArguments)
{
    auto pack = stew::PackagedArguments(std::string("one"), 2, 3.3f);

    EXPECT_CALL(*m_mockPrinter, log("one, 2"));
    invokeFunction(lambda2, pack);
}

TEST_F(PackagedArguments, invokeFunctor)
{
    Functor3 functor = lambda3;
    auto pack = stew::PackagedArguments(std::string("one"), 2, 3.3f);

    EXPECT_CALL(*m_mockPrinter, log("one, 2, 3.3"));
    invokeFunction(functor, pack);
}

TEST_F(PackagedArguments, invokeFunctorWithLesserArguments)
{
    Functor2 functor = lambda2;
    auto pack = stew::PackagedArguments(std::string("one"), 2, 3.3f);

    EXPECT_CALL(*m_mockPrinter, log("one, 2"));
    invokeFunction(functor, pack);
}

TEST_F(PackagedArguments, invokeFunction)
{
    auto pack = stew::PackagedArguments(std::string("one"), 2, 3.3f);

    EXPECT_CALL(*m_mockPrinter, log("one, 2, 3.3"));
    invokeFunction(testFunction3, pack);
}

TEST_F(PackagedArguments, invokeFunctionWithLesserArguments)
{
    auto pack = stew::PackagedArguments(std::string("one"), 2, 3.3f);

    EXPECT_CALL(*m_mockPrinter, log("one, 2"));
    invokeFunction(testFunction2, pack);
}

TEST_F(PackagedArguments, invokeMethod)
{
    auto object = Class();
    auto pack = stew::PackagedArguments(&object, std::string("one"), 2, 3.3f);

    EXPECT_CALL(*m_mockPrinter, log("one, 2, 3.3"));
    invokeFunction(&Class::method, pack);
}

TEST_F(PackagedArguments, invokeMethodWithLesserArguments)
{
    auto object = Class();
    auto pack = stew::PackagedArguments(&object, std::string("one"), 2, 3.3f);

    EXPECT_CALL(*m_mockPrinter, log("one, 2"));
    invokeFunction(&Class::method2, pack);
}
