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

#include <stew/dynamic_type/variable.hpp>

using namespace std::string_literals;

namespace
{

class VariableTestBase : public DomainTestEnvironment
{
protected:
    void SetUp() override
    {
        // Single threaded
        initializeDomain(false, true);
        stew::TypeRegistry::instance();
    }

    void TearDown() override
    {
        stew::TypeRegistry::instance().uninitialize();
        DomainTestEnvironment::TearDown();
    }
};

using VariableTests = VariableTestBase;

enum class OpCode
{
    Add,
    Sub,
    Mul,
    Div,
    Eq,
    Less,
    Leq,
    Gt,
    Geq,
    BwAnd,
    BwOr,
    BwXor,
    BwNot,
    BwShl,
    BwShr,
    Ptr,
    CPtr,
};


struct OpTest
{
    stew::Variable lhs;
    stew::Variable rhs;
    stew::Variable expected;
    OpCode op;
    bool throws;
};

class OpTestRunner : public VariableTestBase, public ::testing::WithParamInterface<OpTest>
{
protected:
    OpTest param;
    explicit OpTestRunner()
    {
        this->param = GetParam();
    }
    stew::Variable runOp() const
    {
        if (this->param.op == OpCode::Add)
        {
            return this->param.lhs + this->param.rhs;
        }
        return {};
    }
};

}

TEST_F(VariableTests, unasigned)
{
    stew::Variable var;
    EXPECT_THROW(var.type(), std::bad_typeid);
}

TEST_F(VariableTests, assignToUnassigned)
{
    stew::Variable var;
    var = "delta"s;
    EXPECT_TRUE(var.isTypeOf<std::string>());
}

TEST_F(VariableTests, create_fromAny)
{
    auto var = stew::Variable(std::any(7));
    EXPECT_EQ(typeid(int), var.type());
}

TEST_F(VariableTests, create_fromType)
{
    auto var = stew::Variable(7);
    EXPECT_EQ(typeid(int), var.type());
}

TEST_F(VariableTests, assign_fromAny)
{
    stew::Variable var = std::any(10);
    EXPECT_TRUE(var.isTypeOf<int>());
}

TEST_F(VariableTests, assign_fromType)
{
    stew::Variable var = 10;
    EXPECT_TRUE(var.isTypeOf<int>());
}

TEST_F(VariableTests, reAssignWithDifferentType)
{
    stew::Variable var = 10;
    EXPECT_TRUE(var.isTypeOf<int>());

    var = "alpha"s;
    EXPECT_TRUE(var.isTypeOf<std::string>());
}

// Converters
TEST_F(VariableTests, boolToChar)
{
    stew::Variable var = true;
    std::string s = var;
    EXPECT_EQ("1", s);
}

// Operators
TEST_F(VariableTests, addition)
{
    stew::Variable var = 10;
    var = var + "12"s;

    EXPECT_EQ(22, int(var));
}

INSTANTIATE_TEST_SUITE_P(VariableOperationTest,
                         OpTestRunner,
                         ::testing::Values(OpTest{.lhs=int(10), .rhs=int(12), .expected=int(22), .op=OpCode::Add, .throws=false}));
TEST_P(OpTestRunner, run)
{
    EXPECT_EQ(this->param.expected, this->runOp());
}

