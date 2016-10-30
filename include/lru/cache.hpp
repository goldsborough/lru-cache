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
    assert(iterator != _cache.end());

    _last_accessed = iterator;

    return iterator->second.value;
  }

  const Value &insert(const Key &key, const Value &value) override {
    auto iterator = _cache.find(key);

    if (iterator != _cache.end()) {
      _order.erase(iterator->second.order);

      // Insert and get the iterator (push_back returns
      // void and emplace_back returns a reference ...)
      auto new_order = _order.insert(_order.end(), key);
      iterator->second.order = new_order;

      return iterator->second.value;
    } else {
      if (is_full()) {
        super::_erase_lru();
      }

      auto order = _order.insert(_order.end(), key);
      InformationArguments arguments(value, order);
      auto result = _cache.emplace(key, arguments);

      return result.first->second.value;
    }
  }
};
}
#endif /* LRU_CACHE_HPP*/
