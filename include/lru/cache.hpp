/// The MIT License (MIT)
/// Copyright (c) 2016 Peter Goldsborough and Markus Engel
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to
/// deal in the Software without restriction, including without limitation the
/// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
/// sell copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE.

#ifndef LRU_CACHE_HPP
#define LRU_CACHE_HPP

#include <cassert>
#include <chrono>
#include <cstddef>
#include <functional>
#include <iterator>
#include <list>
#include <stdexcept>
#include <unordered_map>

#include "lru/error.hpp"
#include "lru/internal/base-cache.hpp"
#include "lru/internal/information.hpp"
#include "lru/internal/last-accessed.hpp"

namespace LRU {
namespace Internal {
template <typename Key,
          typename Value,
          typename HashFunction,
          typename KeyEqual>
using UntimedCacheBase = Internal::
    BaseCache<Key, Value, Internal::Information, HashFunction, KeyEqual>;
}

template <typename Key,
          typename Value,
          typename HashFunction = std::hash<Key>,
          typename KeyEqual = std::equal_to<Key>>
class Cache
    : public Internal::UntimedCacheBase<Key, Value, HashFunction, KeyEqual> {
 private:
  using super = Internal::UntimedCacheBase<Key, Value, HashFunction, KeyEqual>;
  using PRIVATE_BASE_CACHE_MEMBERS;

 public:
  using PUBLIC_BASE_CACHE_MEMBERS;
  using typename super::size_t;

  explicit Cache(size_t capacity = Internal::DEFAULT_CAPACITY,
                 const HashFunction& hash = HashFunction(),
                 const KeyEqual& equal = KeyEqual())
  : super(capacity, hash, equal) {
  }

  template <typename Iterator>
  Cache(size_t capacity,
        Iterator begin,
        Iterator end,
        const HashFunction& hash = HashFunction(),
        const KeyEqual& equal = KeyEqual())
  : super(capacity, begin, end, hash, equal) {
  }

  template <typename Iterator>
  Cache(Iterator begin,
        Iterator end,
        const HashFunction& hash = HashFunction(),
        const KeyEqual& equal = KeyEqual())
  : super(begin, end, hash, equal) {
  }

  template <typename Range>
  explicit Cache(Range&& range,
                 const HashFunction& hash = HashFunction(),
                 const KeyEqual& equal = KeyEqual())
  : super(std::forward<Range>(range), hash, equal) {
  }

  template <typename Range>
  Cache(size_t capacity,
        Range&& range,
        const HashFunction& hash = HashFunction(),
        const KeyEqual& equal = KeyEqual())
  : super(capacity, std::forward<Range>(range), hash, equal) {
  }

  Cache(InitializerList list,
        const HashFunction& hash = HashFunction(),
        const KeyEqual& equal = KeyEqual())  // NOLINT(runtime/explicit)
      : super(list, hash, equal) {
  }

  Cache(size_t capacity,
        InitializerList list,
        const HashFunction& hash = HashFunction(),
        const KeyEqual& equal = KeyEqual())  // NOLINT(runtime/explicit)
      : super(capacity, list, hash, equal) {
  }

  UnorderedIterator find(const Key& key) override {
    auto iterator = _cache.find(key);
    if (iterator != _cache.end()) {
      _last_accessed = iterator;
    }

    return {*this, iterator};
  }

  UnorderedConstIterator find(const Key& key) const override {
    auto iterator = _cache.find(key);
    if (iterator != _cache.end()) {
      _last_accessed = iterator;
    }

    return {*this, iterator};
  }

  const Key& front() const noexcept {
    if (is_empty()) {
      throw LRU::Error::EmptyCache("front");
    } else {
      return _order.front();
    }
  }

  const Key& back() const noexcept {
    if (is_empty()) {
      throw LRU::Error::EmptyCache("back");
    } else {
      return _order.back();
    }
  }
};
}  // namespace LRU

#endif  // LRU_CACHE_HPP
