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

#include "lru/internal/base-cache.hpp"
#include "lru/internal/information.hpp"
#include "lru/internal/last-accessed.hpp"

namespace LRU {
namespace Internal {
template <typename Key, typename Value>
using UntimedCacheBase = Internal::BaseCache<Key, Value, Internal::Information>;
}

template <typename Key, typename Value>
class Cache : public Internal::UntimedCacheBase<Key, Value> {
 private:
  using super = Internal::UntimedCacheBase<Key, Value>;
  using super::_cache;
  using super::_order;
  using super::_last_accessed;
  using super::_capacity;
  using super::is_full;
  using super::_erase_lru;
  using super::is_empty;
  using super::_move_to_front;
  using typename super::Information;

 public:
  using size_t = std::size_t;

  explicit Cache(size_t capacity = Internal::DEFAULT_CAPACITY)
  : super(capacity) {
  }

  bool contains(const Key& key) override {
    if (_last_accessed == key) return true;

    auto iterator = _cache.find(key);
    if (iterator != _cache.end()) {
      _last_accessed = iterator;
      return true;
    }

    return false;
  }

  const Value& find(const Key& key) const override {
    if (key == _last_accessed) {
      return _last_accessed->value;
    }

    auto iterator = _cache.find(key);

    // Throw error
    assert(iterator != _cache.end());

    _last_accessed = iterator;

    return iterator->second.value;
  }

  const Value& insert(const Key& key, const Value& value) override {
    auto iterator = _cache.find(key);

    // To insert, we first check if the key is already present in the cache
    // and if so, update its value and move its order iterator to the front
    // of the queue. Else, we insert the key at the end of the queue and
    // possibly pop the front if the cache has reached its capacity.

    if (iterator != _cache.end()) {
      _move_to_front(iterator, value);
    } else {
      if (is_full()) {
        super::_erase_lru();
      }

      auto order = _order.insert(_order.end(), key);
      auto result = _cache.emplace(key, Information(value, order));
      assert(result.second);

      _last_accessed = result.first;
    }

    return value;
  }

  const Key& front() const noexcept {
    // throw
    assert(!is_empty());
    return _order.front();
  }

  const Key& back() const noexcept {
    // throw
    assert(!is_empty());
    return _order.back();
  }
};
}
#endif /* LRU_CACHE_HPP*/