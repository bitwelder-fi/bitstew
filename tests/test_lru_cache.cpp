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

#include <meta/cache/lru_cache.hpp>

#include <memory>
#include <string>

namespace
{

struct UserType {};

class META_API TestClock
{
public:
    using duration      = std::size_t;
    using time_point    = std::size_t;

    inline static auto msecs(std::size_t ms)
    {
        return ms;
    }

    inline static auto now()
    {
        return instance().m_count++;
    }

    // Local test API
    static TestClock& instance()
    {
        static TestClock inst;
        return inst;
    }

    void sleep(duration msecs)
    {
        m_count += msecs;
    }

    void reset()
    {
        m_count = 0u;
    }

private:
    std::size_t m_count = 0u;
};

#ifdef META_TEST_MOCK_TTL_CLOCK

using CacheClock = TestClock;

#else

using CacheClock = meta::TtlClock;

#endif

class LruCacheTestBase : public DomainTestEnvironment
{
protected:
    void SetUp() override
    {
        initializeDomain(true, true);
    }
};

class LruCacheTests : public LruCacheTestBase
{
protected:
    using TestCache = meta::LruCache<int, int, meta::no_lock, CacheClock>;

    void SetUp() override
    {
        LruCacheTestBase::SetUp();
        if constexpr (std::is_same_v<TestClock, CacheClock>)
        {
            TestClock::instance().reset();
        }
    }

    void sleep(std::size_t msecs)
    {
        if constexpr (std::is_same_v<CacheClock, TestClock>)
        {
            TestClock::instance().sleep(TestClock::msecs(msecs));
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(msecs));
        }
    }
};

template <class TestTraits>
class LruCacheCreateTests : public LruCacheTestBase
{
protected:
    using key_type = typename TestTraits::key_type;
    using cached_type = typename TestTraits::cached_type;
    using CacheType = meta::LruCache<key_type, cached_type>;
};

template <class Key, class Element>
struct LruTraits
{
    using key_type = Key;
    using cached_type = Element;
};

}

using LruCacheTypes = ::testing::Types<
    LruTraits<int, int>,
    LruTraits<std::string, int>,
    LruTraits<int, std::string>,
    LruTraits<std::string, std::string>,
    LruTraits<std::string, std::shared_ptr<UserType>>
    >;
TYPED_TEST_SUITE(LruCacheCreateTests, LruCacheTypes);

TYPED_TEST(LruCacheCreateTests, create)
{
    constexpr auto ttl = std::chrono::milliseconds(10);
    typename TestFixture::CacheType cache(5u, ttl);

    EXPECT_EQ(5u, cache.capacity());
    EXPECT_EQ(ttl, cache.ttl());
}


TEST_F(LruCacheTests, put_whenSpaceEnough)
{
    TestCache cache(3, CacheClock::msecs(100));

    EXPECT_TRUE(cache.put(1, 101));
    EXPECT_EQ(1u, cache.size());
}

TEST_F(LruCacheTests, put_whenKeyIsSame)
{
    TestCache cache(3, CacheClock::msecs(100));

    EXPECT_TRUE(cache.put(1, 101));
    EXPECT_TRUE(cache.put(2, 102));
    EXPECT_TRUE(cache.put(3, 103));
    EXPECT_TRUE(cache.put(1, 104));
    EXPECT_EQ(3u, cache.size());
}

TEST_F(LruCacheTests, put_failsWhenCapacityReachedAndNoExpiredKeys)
{
    TestCache cache(3, CacheClock::msecs(100));

    EXPECT_TRUE(cache.put(1, 101));
    EXPECT_TRUE(cache.put(2, 102));
    EXPECT_TRUE(cache.put(3, 103));
    EXPECT_FALSE(cache.put(4, 104));
    EXPECT_EQ(3u, cache.size());
}

TEST_F(LruCacheTests, put_succeedsWithExpiredKeys)
{
    TestCache cache(3, CacheClock::msecs(20));

    EXPECT_TRUE(cache.put(1, 101));
    sleep(5);
    EXPECT_TRUE(cache.put(2, 102));
    sleep(5);
    EXPECT_TRUE(cache.put(3, 103));

    // Delay to get expired keys.
    sleep(10);
    EXPECT_TRUE(cache.put(4, 104));
}

TEST_F(LruCacheTests, put_succeedsWithAllKeysExpired)
{
    TestCache cache(3, CacheClock::msecs(20));

    EXPECT_TRUE(cache.put(1, 101));
    sleep(5);
    EXPECT_TRUE(cache.put(2, 102));
    sleep(5);
    EXPECT_TRUE(cache.put(3, 103));
    // Delay to get expired keys.
    sleep(20);
    EXPECT_TRUE(cache.put(4, 104));
    EXPECT_EQ(1u, cache.size());
}

TEST_F(LruCacheTests, getContent)
{
    TestCache cache(3, CacheClock::msecs(20));
    std::vector<std::pair<int, int>> contentMatch({{2, 102}, {3, 103}, {4, 104}});

    EXPECT_TRUE(cache.put(1, 101));
    sleep(5);
    EXPECT_TRUE(cache.put(2, 102));
    sleep(5);
    EXPECT_TRUE(cache.put(3, 103));
    // Delay to get expired keys.
    sleep(10);
    EXPECT_TRUE(cache.put(4, 104));

    EXPECT_EQ(contentMatch, cache.getContent());
}

TEST_F(LruCacheTests, get_savesFromExpiry)
{
    TestCache cache(3, CacheClock::msecs(20));
    std::vector<std::pair<int, int>> contentMatch({{2, 102}, {3, 103}, {1, 101}});
    std::vector<std::pair<int, int>> contentMatch2({{3, 103}, {1, 101}});

    EXPECT_TRUE(cache.put(1, 101));
    sleep(5);
    EXPECT_TRUE(cache.put(2, 102));
    sleep(5);
    EXPECT_TRUE(cache.put(3, 103));

    EXPECT_NE(std::nullopt, cache.get(1));
    EXPECT_EQ(contentMatch, cache.getContent());

    // Delay a bit more to get expired elements
    sleep(15);
    EXPECT_EQ(contentMatch2, cache.getContent());
}

TEST_F(LruCacheTests, purge)
{
    TestCache cache(3, CacheClock::msecs(20));

    EXPECT_TRUE(cache.put(1, 101));
    EXPECT_TRUE(cache.put(2, 102));
    EXPECT_TRUE(cache.put(3, 103));
    EXPECT_EQ(3u, cache.size());

    sleep(20);
    EXPECT_EQ(0u, cache.size());
    EXPECT_EQ(3u, cache.getElementCount());
    cache.purge();
    EXPECT_EQ(0u, cache.getElementCount());
}

TEST_F(LruCacheTests, clear)
{
    TestCache cache(3, CacheClock::msecs(20));

    EXPECT_TRUE(cache.put(1, 101));
    EXPECT_TRUE(cache.put(2, 102));
    EXPECT_TRUE(cache.put(3, 103));
    EXPECT_EQ(3u, cache.size());
    EXPECT_EQ(3u, cache.getElementCount());

    cache.clear();
    EXPECT_EQ(0u, cache.size());
    EXPECT_EQ(0u, cache.getElementCount());
}

