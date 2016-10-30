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

#include "lru/internal/last-accessed.hpp"

namespace LRU {

template <typename Key, typename Value>
class Cache {
 public:
  using size_t = std::size_t;

  explicit Cache(size_t time_to_live,
                 size_t capacity = Internal::Globals::DEFAULT_CAPACITY)
  : _capacity(capacity), _time_to_live(time_to_live) {
  }

  bool contains(const Key &key) {
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

  const Value &find(const Key &key) const {
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

  const Value &operator[](const Key &key) const {
    return find(key);
  }

  void insert(const Key &key, const Value &value) {
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

  void erase(const Key &key) {
    if (_last_accessed == key) {
      _erase(_last_accessed.iterator());
    }

    auto iterator = _cache.find(key);

    if (iterator != _cache.end()) {
      _erase(iterator);
    }
  }

  size_t size() const noexcept {
    return _cache.size();
  }

  size_t capacity() const noexcept {
    return _capacity;
  }

  size_t space_left() const noexcept {
    return _capacity - size();
  }

  bool is_empty() const noexcept {
    return size() == 0;
  }

  bool is_full() const noexcept {
    return size() == _capacity;
  }

 private:
  using Clock = std::chrono::steady_clock;
  using Timestamp = Clock::time_point;
  using Queue = std::list<Key>;
  using QueueIterator = typename Queue::const_iterator;

  // Make it a friend class so it has access to the internals
  // of the cache, for faster operations
  template <typename Function, typename... Keys>
  friend class MemoizedFunction;

  struct Information {
    using Arguments = std::pair<const Value &, QueueIterator>;

    Information(const Value &value_, QueueIterator order_)
    : value(value_), insertion_time(Clock::now()), order(order_) {
    }

    Information(const Arguments &arguments)
    : Information(arguments.first, arguments.second) {
    }

    const Value value;
    const Timestamp insertion_time;
    QueueIterator order;
  };

  using Map = std::unordered_map<Key, Information>;
  using MapIterator = typename Map::iterator;

  void _erase_lru() {
    auto lru = _order.front();
    _order.pop_front();
    _cache.erase(lru);
  }

  void _erase(MapIterator iterator) {
    if (_last_accessed == iterator) {
      _last_accessed.invalidate();
    }

    _order.erase(iterator->second.order);
    _cache.erase(iterator);
  }

  bool _has_time_to_live(const Information &information) const noexcept {
    auto elapsed = Clock::now() - information.insertion_time;
    return std::chrono::duration_cast<std::chrono::milliseconds>(elapsed)
               .count() < _time_to_live;
  }

  Map _cache;
  Queue _order;

  mutable Internal::LastAccessed<MapIterator> _last_accessed;

  size_t _capacity;
  size_t _time_to_live;
};
}
#endif /* LRU_CACHE_HPP*/
