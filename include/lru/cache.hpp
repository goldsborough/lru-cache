/**
* The MIT License (MIT)
* Copyright (c) 2016 Peter Goldsborough and Markus Engel
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#ifndef LRU_CACHE_HPP
#define LRU_CACHE_HPP

#include <cassert>
#include <chrono>
#include <cstddef>
#include <iterator>
#include <list>
#include <stdexcept>
#include <unordered_map>
#include <utility>

#include "lru/internal/abstract-cache.hpp"
#include "lru/internal/information.hpp"
#include "lru/internal/last-accessed.hpp"

namespace LRU {

template <typename Key, typename Value>
using CacheBase = AbstractCache<Key, Value, Internal::Information>;

template <typename Key, typename Value>
class Cache : public CacheBase<Key, Value> {
 public:
  using size_t = std::size_t;
  using super = CacheBase<Key, Value>;
  using super::_cache;
  using super::_order;
  using super::_last_accessed;
  using super::_capacity;
  using super::is_full;
  using super::_erase_lru;
  using typename super::Information;
  using typename super::InformationArguments;

  explicit Cache(size_t capacity = Internal::DEFAULT_CAPACITY)
  : super(capacity) {
  }

  bool contains(const Key &key) override {
    if (_last_accessed == key) return true;

    auto iterator = _cache.find(key);
    if (iterator != _cache.end()) {
      _last_accessed = iterator;
      return true;
    }

    return false;
  }

  const Value &find(const Key &key) const override {
    if (key == _last_accessed) {
      return _last_accessed->value;
    }

    auto iterator = _cache.find(key);

    // Throw error
    assert(iterator != _cache.end());

    _last_accessed = iterator;

    return iterator->second.value;
  }

  const Value &insert(const Key &key, const Value &value) override {
    auto iterator = _cache.find(key);

    // To insert, we first check if the key is already present in the cache
    // and if so, update its value and move its order iterator to the front
    // of the queue. Else, we insert the key at the end of the queue and
    // possibly pop the front if the cache has reached its capacity.

    if (iterator != _cache.end()) {
      _order.erase(iterator->second.order);

      // Insert and get the iterator (push_back returns
      // void and emplace_back returns a reference ...)
      auto new_order = _order.insert(_order.end(), key);

      iterator->second.order = new_order;
      iterator->second.value = value;
    } else {
      if (is_full()) {
        super::_erase_lru();
      }

      auto order = _order.insert(_order.end(), key);
      InformationArguments arguments(value, order);
      auto result = _cache.emplace(key, arguments);
    }

    return value;
  }
};
}
#endif /* LRU_CACHE_HPP*/
