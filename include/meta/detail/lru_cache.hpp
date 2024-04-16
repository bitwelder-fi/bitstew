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

#ifndef META_DETAIL_LRU_CACHE_HPP
#define META_DETAIL_LRU_CACHE_HPP

#include <meta/meta_api.hpp>
#include <meta/utility/scope_value.hpp>

#include <chrono>
#include <cstdlib>
#include <map>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>

namespace meta
{

using TtlClock = std::chrono::steady_clock;

namespace detail
{

template <class Key, class Element>
struct META_TEMPLATE_API TtlCache
{
    using TimePoint = TtlClock::time_point;
    struct META_TEMPLATE_API CacheNode
    {
        Element element;
        TimePoint expiryTime;

        CacheNode(const Element& element) :
            element(element),
            expiryTime(TtlClock::now())
        {
        }
        CacheNode(Element&& element) :
            element(std::forward<Element>(element)),
            expiryTime(TtlClock::now())
        {
        }

        void update()
        {
            expiryTime = TtlClock::now();
        }
    };

    const std::size_t capacity;
    const TtlClock::duration ttl;

    explicit TtlCache(std::size_t capacity, TtlClock::duration ttl) :
        capacity(capacity),
        ttl(ttl)
    {
    }

    bool put(const Key& key, CacheNode&& node)
    {
        if (auto cacheIt = m_cache.find(key); cacheIt != m_cache.end())
        {
            m_timeBuffer.erase(cacheIt->second.expiryTime);
            m_timeBuffer.insert(std::make_pair(node.expiryTime, key));
            cacheIt->second = std::move(node);
            return true;
        }

        if (m_cache.size() < capacity)
        {
            const auto expiryTime = node.expiryTime;
            m_cache.insert(std::make_pair(key, std::forward<CacheNode>(node)));
            m_timeBuffer.insert(std::make_pair(expiryTime, key));
            return true;
        }

        // Eviction. Purge the expired elements, and retry.
        if (purgeOne())
        {
            const auto expiryTime = node.expiryTime;
            m_cache.insert(std::make_pair(key, std::forward<CacheNode>(node)));
            m_timeBuffer.insert(std::make_pair(expiryTime, key));
            return true;
        }

        return false;
    }

    std::optional<Element> get(const Key& key)
    {
        if (auto cacheIt = m_cache.find(key); cacheIt != m_cache.end())
        {
            // TODO: if the value had expired, remove and leave.
            m_timeBuffer.erase(cacheIt->second.expiryTime);
            cacheIt->second.update();
            m_timeBuffer.insert(std::make_pair(cacheIt->second.expiryTime, key));
            return cacheIt->second.element;
        }
        return std::nullopt;
    }

    void purge()
    {
        const auto timeStamp = TtlClock::now() - ttl;
        auto pos = m_timeBuffer.upper_bound(timeStamp);
        if (pos == m_timeBuffer.end())
        {
            // All expired.
            clear();
            return;
        }

        for (auto it = m_timeBuffer.begin(); it != pos; ++it)
        {
            m_cache.erase(it->second);
        }
        // Remove time stamps.
        m_timeBuffer.erase(m_timeBuffer.begin(), pos);
    }

    void clear()
    {
        m_cache.clear();
        m_timeBuffer.clear();
    }

    std::size_t size() const
    {
        const auto timeStamp = TtlClock::now() - ttl;
        auto first = m_timeBuffer.upper_bound(timeStamp);
        return std::distance(first, m_timeBuffer.end());
    }

    std::vector<std::pair<Key, Element>> content() const
    {
        const auto timeStamp = TtlClock::now() - ttl;
        auto begin = m_timeBuffer.upper_bound(timeStamp);

        std::vector<std::pair<Key, Element>> result;
        for (auto it = begin; it != m_timeBuffer.end(); ++it)
        {
            auto node = m_cache.find(it->second);
            result.push_back(std::make_pair(node->first, node->second.element));
        }
        return result;
    }

private:

    bool purgeOne()
    {
        const auto timeStamp = TtlClock::now() - ttl;
        auto pos = m_timeBuffer.upper_bound(timeStamp);

        if (pos == m_timeBuffer.begin())
        {
            // Nothing expired.
            return false;
        }

        // Remove the first element.
        m_cache.erase(m_timeBuffer.begin()->second);
        m_timeBuffer.erase(m_timeBuffer.begin());
        return true;
    }

    using CacheContainer = std::unordered_map<Key, CacheNode>;
    using TtlKeys = std::map<TimePoint, Key>;

    CacheContainer m_cache;
    TtlKeys m_timeBuffer;
};

}}

#endif
