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

#ifndef LRU_MEMOIZE_HPP
#define LRU_MEMOIZE_HPP

#include <cstddef>
#include <tuple>
#include <unordered_map>
#include <utility>

#include "lru/cache.hpp"
#include "lru/errors.hpp"
#include "lru/internal/definitions.hpp"
#include "lru/internal/hash.hpp"
#include "lru/statistics.hpp"

namespace LRU {

template <typename Function, typename... Keys>
class MemoizedFunction {
 public:
  using size_t = std::size_t;
  using Key = std::tuple<Keys...>;
  using Value = decltype((Function*){}(Keys{}...));  // NOLINT
  using InitializerList = std::initializer_list<Key>;

 private:
  using HitMap = std::unordered_map<Key, size_t>;

 public:
  MemoizedFunction(const Function& function,
                   size_t capacity,
                   InitializerList list)
  : _function(function), _cache(capacity), _total_hits(0), _accesses(0) {
    for (const auto& key : list) {
      _element_hits.emplace(key, 0);
    }
  }

  template <typename... InitializerKeys>
  MemoizedFunction(const Function& function,
                   size_t capacity,
                   InitializerKeys&&... keys)
  : _function(function), _cache(capacity), _total_hits(0), _accesses(0) {
  }

  template <typename... Rest>
  const Value& operator()(Keys&&... keys, Rest&&... rest) {
    ++_accesses;

    auto key_tuple = std::make_tuple(std::forward<Keys>(keys)...);

    if (_cache.contains(key_tuple)) {
      ++_total_hits;
      _monitor_key(key_tuple);

      return _cache[key_tuple];
    }

    // clang-format off
    auto result = _function(
          std::forward<Keys>(keys)...,
          std::forward<Rest>(rest)...
    );
    // clang-format on

    return _cache.insert(key_tuple, result);
  }

  size_t hit_rate() const noexcept {
    return static_cast<double>(_total_hits) / _accesses;
  }

  size_t hits_for(const Key& key) const {
    auto iterator = _element_hits.find(key);
    if (iterator == _element_hits.end()) {
      throw NotMonitoredError();
    }
    return _element_hits.at(key);
  }

  size_t total_accesses() const noexcept {
    return _accesses;
  }

  size_t total_hits() const noexcept {
    return _total_hits;
  }

  size_t total_misses() const noexcept {
    return total_accesses() - total_hits();
  }

  Statistics<HitMap> statistics() const noexcept {
    // clang-format off
    return {
        total_hits(),
        _element_hits,
        total_accesses(),
        total_hits(),
        total_misses()
    };
    // clang-format on
  }


 private:
  using Cache = LRU::Cache<Key, Value>;

  void _monitor_key(const Key& key) {
    auto element_iterator = _element_hits.find(key);
    if (element_iterator != _element_hits.end()) {
      ++(element_iterator->second);
    }
  }

  const Function& _function;
  Cache _cache;
  HitMap _element_hits;

  size_t _total_hits;
  size_t _accesses;
};


template <typename... Keys, typename Function>
static auto memoize(const Function& function, size_t capacity, Keys&&... keys) {
  // clang-format off
  return MemoizedFunction<Function, Keys...>(
      function,
      capacity,
      std::forward<Keys>(keys)...
  );  // NOLINT(whitespace/parens)
  // clang-format on
}

template <typename... Keys>
using MonitorList = std::initializer_list<std::tuple<Keys...>>;

template <typename... Keys, typename Function>
static auto
memoize(const Function& function,
        size_t capacity = Internal::DEFAULT_CAPACITY,
        MonitorList<Keys...> keys_to_monitor = MonitorList<Keys...>()) {
  // clang-format off
  return MemoizedFunction<Function, Keys...>(
      function,
      capacity,
      keys_to_monitor
  );  // NOLINT(whitespace/parens)
  // clang-format on
}
}

#endif /* LRU_MEMOIZE_HPP*/
