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

#include <containers/guarded_sequence_container.hpp>

#include <concepts>
#include <cstdlib>
#include <deque>
#include <limits>
#include <list>
#include <memory>
#include <utils/concepts.hpp>
#include <vector>

namespace
{

enum class EnumType
{
    Invalid,
    One,
    Two,
    Three
};

template <class E>
concept test_enum = std::is_enum_v<E> && std::is_same_v<E, EnumType>;

struct UserType
{
};
template <class T>
concept user_type = std::is_same_v<T, UserType>;

template <typename BaseContainerType>
class GuardedSequenceContainerTests : public DomainTestEnvironment
{
protected:
    using value_type = typename BaseContainerType::value_type;
    using GuardedContainer = containers::GuardedSequenceContainer<BaseContainerType>;
    using size_type = typename GuardedContainer::size_type;
    using Guard = containers::LockView<GuardedContainer>;

    std::unique_ptr<GuardedContainer> m_container;
    size_type m_initialSize;

    void SetUp() override
    {
        initializeDomain(false, true);

        m_container = std::make_unique<GuardedContainer>(getInvalidElement());
        m_initialSize = m_container->size();
    }

    void fillUp(std::size_t count)
    {
        MAYBE_UNUSED(count);
        for (auto i = 0u; i != count; ++i)
        {
            if constexpr(std::is_arithmetic_v<value_type>)
            {
                this->m_container->push_back(static_cast<value_type>(i));
            }
            else
            {
                this->m_container->push_back(generateValue());
            }
        }
    }

    auto getInvalidElement() const
        requires std::signed_integral<value_type>
    {
        return std::numeric_limits<value_type>::min();
    }
    auto getInvalidElement() const
        requires std::unsigned_integral<value_type>
    {
        return std::numeric_limits<value_type>::max();
    }
    auto getInvalidElement() const
        requires std::floating_point<value_type>
    {
        return std::numeric_limits<value_type>::quiet_NaN();
    }
    auto getInvalidElement() const
        requires concepts::raw_pointer<value_type>
    {
        return nullptr;
    }
    auto getInvalidElement() const
        requires concepts::smart_pointer<value_type>
    {
        return value_type();
    }
    auto getInvalidElement() const
        requires test_enum<value_type>
    {
        return EnumType::Invalid;
    }
    auto getInvalidElement() const
        requires concepts::std_string<value_type>
    {
        return std::string("");
    }
    auto getInvalidElement() const
        requires concepts::std_string_view<value_type>
    {
        return std::string_view("");
    }

    value_type generateValue() const
        requires std::is_arithmetic_v<value_type>
    {
        const auto invalid = getInvalidElement();
        auto element = static_cast<value_type>(rand());
        for (;element == invalid; element = static_cast<value_type>(rand()));
        return element;
    }
    value_type generateValue() const
        requires std::is_pointer_v<value_type>
    {
        return new value_type();
    }
    value_type generateValue() const
        requires traits::is_unique_pointer_v<value_type>
    {
        using data_type = typename value_type::element_type;
        return std::make_unique<data_type>();
    }
    value_type generateValue() const
        requires traits::is_shared_pointer_v<value_type>
    {
        using data_type = typename value_type::element_type;
        return std::make_shared<data_type>();
    }
    value_type generateValue() const
        requires std::is_same_v<EnumType, value_type>
    {
        return EnumType::Three;
    }
    value_type generateValue() const
        requires traits::is_std_string_v<value_type>
    {
        return std::to_string(rand());
    }
    value_type generateValue() const
        requires traits::is_std_string_view_v<value_type>
    {
        return std::string_view("test");
    }
};

}

using ContainerTypes = ::testing::Types<
    std::vector<int>,
    std::vector<float>,
    std::vector<char>,
    std::vector<EnumType>,
    std::vector<std::string>,
    std::vector<std::string_view>,
    std::vector<std::shared_ptr<UserType>>,

    std::deque<int>,
    std::deque<float>,
    std::deque<char>,
    std::deque<EnumType>,
    std::deque<std::string>,
    std::deque<std::string_view>,
    std::deque<std::shared_ptr<UserType>>
    >;
TYPED_TEST_SUITE(GuardedSequenceContainerTests, ContainerTypes);

// Size
TYPED_TEST(GuardedSequenceContainerTests, size)
{
    EXPECT_EQ(0u, this->m_initialSize);
}

// Push back
TYPED_TEST(GuardedSequenceContainerTests, push_back)
{
    this->m_container->push_back(this->generateValue());
    EXPECT_GT(this->m_container->size(), this->m_initialSize);
}

TYPED_TEST(GuardedSequenceContainerTests, push_back_whenGuarded)
{
    auto guard = typename TestFixture::Guard(*this->m_container);
    this->m_container->push_back(this->generateValue());
    EXPECT_GT(this->m_container->size(), this->m_initialSize);
}

// Insert
TYPED_TEST(GuardedSequenceContainerTests, insert)
{
    const auto value = this->generateValue();

    auto result = this->m_container->insert(this->m_container->cbegin(), value);
    ASSERT_NE(std::nullopt, result);
    EXPECT_EQ(this->m_container->begin(), *result);
    EXPECT_EQ(this->m_container->size(), this->m_initialSize + 1);
}

TYPED_TEST(GuardedSequenceContainerTests, insertInGuardedAreaWhenEmpty)
{
    auto guard = typename TestFixture::Guard(*this->m_container);
    const auto value = this->generateValue();

    auto result = this->m_container->insert(this->m_container->cbegin(), value);
    ASSERT_NE(std::nullopt, result);
    EXPECT_EQ(this->m_container->begin(), *result);
    EXPECT_EQ(this->m_container->size(), this->m_initialSize + 1);
}

TYPED_TEST(GuardedSequenceContainerTests, insertInGuardedAreaWhenNotEmpty)
{
    this->fillUp(1);
    this->m_initialSize = this->m_container->size();
    auto guard = typename TestFixture::Guard(*this->m_container);
    const auto value = this->generateValue();

    auto result = this->m_container->insert(this->m_container->cbegin(), value);
    ASSERT_EQ(std::nullopt, result);
    EXPECT_EQ(this->m_container->size(), this->m_initialSize);
}

TYPED_TEST(GuardedSequenceContainerTests, insertOuterGuardedAreaWhenNotEmpty)
{
    this->fillUp(1);
    this->m_initialSize = this->m_container->size();
    auto guard = typename TestFixture::Guard(*this->m_container);
    const auto value = this->generateValue();

    auto result = this->m_container->insert(this->m_container->cend(), value);
    ASSERT_NE(std::nullopt, result);
    EXPECT_EQ(this->m_container->size(), this->m_initialSize + 1);
}

TYPED_TEST(GuardedSequenceContainerTests, insertMove)
{
    auto result = this->m_container->insert(this->m_container->cbegin(), std::move(this->generateValue()));
    ASSERT_NE(std::nullopt, result);
    EXPECT_EQ(this->m_container->begin(), *result);
    EXPECT_EQ(this->m_container->size(), this->m_initialSize + 1);
}

TYPED_TEST(GuardedSequenceContainerTests, insertMoveInGuardedAreaWhenEmpty)
{
    auto guard = typename TestFixture::Guard(*this->m_container);
    auto result = this->m_container->insert(this->m_container->cbegin(), std::move(this->generateValue()));
    ASSERT_NE(std::nullopt, result);
    EXPECT_EQ(this->m_container->begin(), *result);
    EXPECT_EQ(this->m_container->size(), this->m_initialSize + 1);
}

TYPED_TEST(GuardedSequenceContainerTests, insertMoveInGuardedAreaWhenNotEmpty)
{
    this->fillUp(1);
    this->m_initialSize = this->m_container->size();
    auto guard = typename TestFixture::Guard(*this->m_container);

    auto result = this->m_container->insert(this->m_container->cbegin(), std::move(this->generateValue()));
    ASSERT_EQ(std::nullopt, result);
    EXPECT_EQ(this->m_container->size(), this->m_initialSize);
}

TYPED_TEST(GuardedSequenceContainerTests, insertMoveOuterGuardedAreaWhenNotEmpty)
{
    this->fillUp(1);
    this->m_initialSize = this->m_container->size();
    auto guard = typename TestFixture::Guard(*this->m_container);

    auto result = this->m_container->insert(this->m_container->cend(), std::move(this->generateValue()));
    ASSERT_NE(std::nullopt, result);
    EXPECT_EQ(this->m_container->size(), this->m_initialSize + 1);
}

// Clear
TYPED_TEST(GuardedSequenceContainerTests, clearEmpty)
{
    EXPECT_EQ(0u, this->m_container->size());
    this->m_container->clear();
    EXPECT_EQ(0u, this->m_container->size());
}

TYPED_TEST(GuardedSequenceContainerTests, clearNotEmpty)
{
    this->fillUp(1);

    EXPECT_NE(0u, this->m_container->size());
    this->m_container->clear();
    EXPECT_EQ(0u, this->m_container->size());
}

TYPED_TEST(GuardedSequenceContainerTests, clearNotEmptyGuarded)
{
    this->fillUp(10);
    auto guard = typename TestFixture::Guard(*this->m_container);

    EXPECT_NE(0u, this->m_container->size());
    this->m_container->clear();
    EXPECT_EQ(0u, this->m_container->size());
}

// Erase
TYPED_TEST(GuardedSequenceContainerTests, eraseNotGuarded)
{
    this->fillUp(10);
    auto pos = this->m_container->cbegin();
    pos += 4;
    auto next = this->m_container->toIterator(pos);
    ++next;
    typename TestFixture::value_type nextValue = *next;

    auto result = this->m_container->erase(pos);
    ASSERT_NE(std::nullopt, result);
    EXPECT_EQ(nextValue, **result);
}

TYPED_TEST(GuardedSequenceContainerTests, eraseInGuardedArea)
{
    this->fillUp(10);
    auto pos = this->m_container->cbegin();
    pos += 4;
    auto next = this->m_container->toIterator(pos);
    ++next;
    typename TestFixture::value_type nextValue = *next;
    auto guard = typename TestFixture::Guard(*this->m_container);

    auto result = this->m_container->erase(pos);
    ASSERT_NE(std::nullopt, result);
    EXPECT_EQ(nextValue, **result);
}

TYPED_TEST(GuardedSequenceContainerTests, eraseOuterGuardedArea)
{
    this->fillUp(10);
    auto guard = typename TestFixture::Guard(*this->m_container);
    this->fillUp(10);
    ASSERT_EQ(20, this->m_container->size());

    auto pos = this->m_container->cbegin();
    pos += 12;

    auto result = this->m_container->erase(pos);
    EXPECT_EQ(std::nullopt, result);
    EXPECT_EQ(19, this->m_container->size());
}
