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

#ifndef LRU_TIMED_CACHE_HPP
#define LRU_TIMED_CACHE_HPP

#include <cassert>
#include <chrono>
#include <cstddef>
#include <iterator>
#include <list>
#include <stdexcept>
#include <unordered_map>
#include <utility>

#include "lru/internal/base-cache.hpp"
#include "lru/internal/last-accessed.hpp"

namespace LRU {
namespace Internal {
template <typename Key, typename Value>
using TimedCacheBase = BaseCache<Key, Value, Internal::TimedInformation>;
}

template <typename Key,
          typename Value,
          typename Duration = std::chrono::milliseconds>
class TimedCache : public Internal::TimedCacheBase<Key, Value> {
 private:
  using super = Internal::TimedCacheBase<Key, Value>;
  using super::_cache;
  using super::_order;
  using super::_last_accessed;
  using super::_capacity;
  using super::is_full;
  using super::_erase_lru;
  using super::_move_to_front;
  using super::is_empty;
  using typename super::Information;

 public:
  using size_t = std::size_t;


  template <typename AnyDurationType>
  explicit TimedCache(const AnyDurationType& time_to_live,
                      size_t capacity = Internal::DEFAULT_CAPACITY)
  : super(capacity)
  , _time_to_live(std::chrono::duration_cast<Duration>(time_to_live)) {
  }

  bool contains(const Key& key) override {
    if (_last_accessed == key) return true;

    auto iterator = _cache.find(key);
    if (iterator != _cache.end()) {
      if (_has_expired(iterator->second)) {
        _last_accessed = iterator;
        return true;
      } else {
        _erase(iterator);
      }
    }

    return false;
  }

  const Value& find(const Key& key) const override {
    if (key == _last_accessed) {
      if (_has_expired(*_last_accessed)) {
        _erase(_last_accessed);
        throw std::out_of_range{"Element has expired."};
      } else {
        return _last_accessed->value;
      }
    }

    auto iterator = _cache.find(key);

    // Throw
    assert(iterator != _cache.end());

    if (_has_expired(*iterator)) {
      _erase(iterator);
      throw std::out_of_range{"Element has expired."};
    }

    _last_accessed = iterator;

    return iterator->second.value;
  }

  void insert(const Key& key, const Value& value) override {
    auto iterator = _cache.find(key);

    if (iterator != _cache.end()) {
      _move_to_front(iterator, value);
    } else {
      if (is_full()) {
        super::_erase_lru();
      }

      auto order = _order.insert(_order.end(), key);
      auto result = _cache.emplace(key, Information(value, order));
      assert(result.first);

      _last_accessed = result.second;
    }
  }

  // no front() because we may have to erase the entire cache if everything
  // happens to be expired

  bool all_expired() const noexcept {
    if (is_empty()) return false;

    auto latest = _cache.find(_order.back());
    return has_expired(*latest);
  }

 private:
  using Clock = typename Internal::Clock;

  bool _has_expired(const Information& information) const noexcept {
    auto elapsed = Clock::now() - information.insertion_time;
    return std::chrono::duration_cast<Duration>(elapsed) > _time_to_live;
  }

  Duration _time_to_live;
};
}  // namespace LRU

#endif /* LRU_TIMED_CACHE_HPP*/
