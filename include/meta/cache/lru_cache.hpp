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

#ifndef META_LRU_CACHE_HPP
#define META_LRU_CACHE_HPP

#include <meta/detail/lru_cache.hpp>
#include <meta/utility/lockable.hpp>

#include <chrono>
#include <utility>

namespace meta
{

/// A Least Recent Used cache with Time To Leave.
///
/// \tparam Key The key type. Must support std::hash().
/// \tparam Element The element type. Must be copy-constructible and copy-assignable.
template <class Key, class ValueType, typename Mutex = meta::mutex>
    requires std::is_copy_constructible_v<ValueType> &&
             std::is_copy_constructible_v<ValueType>
class META_TEMPLATE_API LruCache
{
    using Base = detail::TtlCache<Key, ValueType>;

    Mutex m_mutex;
    Base m_cache;

public:
    /// The key type of the cache.
    using key_type = Key;
    /// The value type of the cached elements.
    using cached_type = ValueType;
    /// The value type, the pair of key_type and cached_type.
    using value_type = std::pair<Key, ValueType>;

    /// Constructor, creates a least recently used cache with a capacity and a TTL duration.
    /// \param capacity The capacity of the cache.
    /// \param ttl The time-to-leave of the cached elements.
    explicit LruCache(std::size_t capacity, TtlClock::duration ttl = std::chrono::milliseconds(3600)) :
        m_cache(capacity, ttl)
    {
    }

    /// Puts a {key, element} pair into the cache.
    /// - If the key exists in the cache, it overrides the element, and updates the expiry time.
    /// - If the key does not exist in the cache, and the size of the cache is less than the capacity,
    ///   the element is added to the cache.
    /// - If the key does not exist in the cache, and the cache reached the capacity, the cache tries
    ///   to purge the expired cache entries, and retries the previous operation.
    /// \param key The key of the cached element.
    /// \param element The element to put in the cache.
    bool put(const key_type& key, const cached_type& element)
    {
        auto cacheNode = typename Base::CacheNode(element);
        std::lock_guard<Mutex> lock(m_mutex);
        return m_cache.put(key, std::move(cacheNode));
    }

    /// Puts a {key, element} pair into the cache. Moves the element into the cache.
    /// - If the key exists in the cache, it overrides the element, and updates the expiry time.
    /// - If the key does not exist in the cache, and the size of the cache is less than the capacity,
    ///   the element is added to the cache.
    /// - If the key does not exist in the cache, and the cache reached the capacity, the cache tries
    ///   to purge the expired cache entries, and retries the previous operation.
    /// \param key The key of the cached element.
    /// \param element The element to put in the cache.
    bool put(const key_type& key, cached_type&& element)
    {
        auto cacheNode = typename Base::CacheNode(std::forward<cached_type>(element));
        std::lock_guard<Mutex> lock(m_mutex);
        return m_cache.put(key, std::move(cacheNode));
    }

    /// Returns the cached element identified by a key.
    /// \param key The key for which to get the cached element.
    /// \return The cached element identified by the key. If the key does not identify a cached element,
    ///         or the cached element expired, returns \e nullopt.
    std::optional<cached_type> get(const key_type& key)
    {
        std::lock_guard<Mutex> lock(m_mutex);
        return m_cache.get(key);
    }

    /// Returns whether the cache is empty.
    bool isEmpty()
    {
        std::lock_guard<Mutex> lock(m_mutex);
        m_cache.purge();
        return m_cache.isEmpty();
    }

    /// Returns the capacity of the cache.
    auto capacity() const
    {
        return m_cache.capacity;
    }

    /// Returns the capacity of the cache.
    auto ttl() const
    {
        return m_cache.ttl;
    }

    /// Returns the number of valid cached elements. The valid cached elements are the non-expired
    /// cached elements.
    std::size_t size()
    {
        std::lock_guard<Mutex> lock(m_mutex);
        return m_cache.cacheCount.load();
    }

    /// Returns the non-expired cached elements.
    /// \return The vector with non-expired elements with keys. Returns an empty vector if the cache
    ///         has no elements or all elements are expired.
    auto getContent()
    {
        std::lock_guard<Mutex> lock(m_mutex);
        m_cache.purge();
        return m_cache.content();
    }

    /// Purges the cache. Removes the expired cache elements.
    void purge()
    {
        std::lock_guard<Mutex> lock(m_mutex);
        m_cache.purge();
    }

    /// Clears the cache.
    void clear()
    {
        std::lock_guard<Mutex> lock(m_mutex);
        m_cache.clear();
    }
};

}

#endif // META_LRU_CACHE_HPP
