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

#include "lru/cache.hpp"
#include "lru/internal/last-accessed.hpp"

namespace LRU {

template <typename Key, typename Value>
using TimedCacheBase = AbstractCache<Key, Value, Internal::TimedInformation>;

template <typename Key, typename Value, typename Duration>
class TimedCache : public TimedCacheBase<Key, Value> {
 public:
  using size_t = std::size_t;
  using super = TimedCacheBase<Key, Value>;
  using super::_cache;
  using super::_order;
  using super::_last_accessed;
  using super::_capacity;
  using super::is_full;
  using super::_erase_lru;
  using typename super::Information;

  explicit TimedCache(Duration time_to_live,
                      size_t capacity = Internal::DEFAULT_CAPACITY)
  : super(capacity), _time_to_live(time_to_live) {
  }

  bool contains(const Key &key) override {
    if (_last_accessed == key) return true;

    auto iterator = _cache.find(key);
    if (iterator != _cache.end()) {
      if (_has_time_to_live(iterator->second)) {
        _last_accessed = iterator;
        return true;
      } else {
        erase(key);
      }
    }

    return false;
  }

  const Value &find(const Key &key) const override {
    if (key == _last_accessed) {
      if (_has_time_to_live(*_last_accessed)) {
        return _last_accessed->value;
      } else {
        _erase(_last_accessed);

        throw std::out_of_range{"Element has expired."};
      }
    }

    auto iterator = _cache.find(key);
    assert(iterator != _cache.end());

    if (!_has_time_to_live(*iterator)) {
      _erase(key);

      throw std::out_of_range{"Element has expired."};
    }

    _last_accessed = iterator;

    return iterator->second.value;
  }

  void insert(const Key &key, const Value &value) override {
    auto iterator = _cache.find(key);

    if (iterator != _cache.end()) {
      _order.erase(iterator->second.order);

      // Insert and get the iterator (push_back returns
      // void and emplace_back returns a reference ...)
      auto new_order = _order.insert(_order.end(), key);
      iterator->second.order = new_order;
    } else {
      if (is_full()) {
        _erase_lru();
      }

      auto order = _order.insert(_order.end(), key);
      auto arguments = typename Information::Arguments{value, order};
      _cache.emplace(key, arguments);
    }
  }


 private:
  using Clock = typename Internal::Clock;

  bool _has_time_to_live(const Information &information) const noexcept {
    auto elapsed = Clock::now() - information.insertion_time;
    return std::chrono::duration_cast<std::chrono::milliseconds>(elapsed)
               .count() < _time_to_live;
  }

  size_t _time_to_live;
};
}

#endif /* LRU_TIMED_CACHE_HPP*/
