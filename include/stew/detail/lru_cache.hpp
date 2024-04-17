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

#ifndef STEW_DETAIL_LRU_CACHE_HPP
#define STEW_DETAIL_LRU_CACHE_HPP

#include <stew/stew_api.hpp>
#include <stew/utility/scope_value.hpp>

#include <cstdlib>
#include <map>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>

namespace stew
{

namespace detail
{

template <class Key, class Element, class Clock>
struct STEW_TEMPLATE_API TtlCache
{
    struct STEW_TEMPLATE_API CacheNode
    {
        Element element;
        Clock::time_point ttl;

        CacheNode(const Element& element, Clock::time_point ttl) :
            element(element),
            ttl(ttl)
        {
        }
        CacheNode(Element&& element, Clock::time_point ttl) :
            element(std::forward<Element>(element)),
            ttl(ttl)
        {
        }
    };

    const std::size_t capacity;
    const Clock::duration ttl;

    explicit TtlCache(std::size_t capacity, Clock::duration ttl) :
        capacity(capacity),
        ttl(ttl)
    {
    }

    bool put(const Key& key, CacheNode&& node)
    {
        if (auto cacheIt = m_cache.find(key); cacheIt != m_cache.end())
        {
            m_timeBuffer.erase(cacheIt->second.ttl);
            m_timeBuffer.insert(std::make_pair(node.ttl, key));
            cacheIt->second = std::move(node);
            return true;
        }

        if (m_cache.size() < capacity)
        {
            const auto ttl = node.ttl;
            m_cache.insert(std::make_pair(key, std::forward<CacheNode>(node)));
            m_timeBuffer.insert(std::make_pair(ttl, key));
            return true;
        }

        // Eviction. Purge the expired elements, and retry.
        if (purgeOne())
        {
            const auto ttl = node.ttl;
            m_cache.insert(std::make_pair(key, std::forward<CacheNode>(node)));
            m_timeBuffer.insert(std::make_pair(ttl, key));
            return true;
        }

        return false;
    }

    std::optional<Element> get(const Key& key)
    {
        if (auto cacheIt = m_cache.find(key); cacheIt != m_cache.end())
        {
            // TODO: if the value had expired, remove and leave.
            m_timeBuffer.erase(cacheIt->second.ttl);
            cacheIt->second.ttl = Clock::now() + ttl;
            m_timeBuffer.insert(std::make_pair(cacheIt->second.ttl, key));
            return cacheIt->second.element;
        }
        return std::nullopt;
    }

    void purge()
    {
        const auto timeStamp = Clock::now();
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
        const auto timeStamp = Clock::now();
        auto first = m_timeBuffer.upper_bound(timeStamp);
        return std::distance(first, m_timeBuffer.end());
    }

    std::size_t getElementCount() const
    {
        return m_cache.size();
    }

    std::vector<std::pair<Key, Element>> content() const
    {
        const auto timeStamp = Clock::now();
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
        const auto timeStamp = Clock::now();
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
    using TtlKeys = std::map<typename Clock::time_point, Key>;

    CacheContainer m_cache;
    TtlKeys m_timeBuffer;
};

}}

#endif
