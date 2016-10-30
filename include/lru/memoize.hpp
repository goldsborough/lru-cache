#ifndef LRU_MEMOIZE_HPP
#define LRU_MEMOIZE_HPP

#include <cstddef>
#include <tuple>
#include <unordered_map>

#include "lru/cache.hpp"
#include "lru/internal/globals.hpp"
#include "lru/internal/hash.hpp"


namespace LRU {

template <typename Function, typename... Keys>
class MemoizedFunction {
 public:
  using size_t = std::size_t;
  using Key = std::tuple<Keys...>;
  using Value = decltype((Function *){}(Keys{}...));
  using InitializerList = std::initializer_list<Key>;

  MemoizedFunction(const Function &function,
                   size_t capacity,
                   InitializerList list)
  : _function(function), _cache(capacity), _overall_hits(0), _accesses(0) {
    for (const auto &key : list) {
      _element_hits.emplace(key);
    }
  }

  template <typename... InitializerKeys>
  MemoizedFunction(const Function &function,
                   size_t capacity,
                   InitializerKeys &&... keys)
  : _function(function), _cache(capacity), _overall_hits(0), _accesses(0) {
  }

  template <typename... Rest>
  auto operator()(Keys &&... keys, Rest &&... rest) {
    ++_accesses;

    auto key_tuple = std::make_tuple(std::forward<Keys>(keys)...);

    auto iterator = _cache._cache.find(key_tuple);
    if (iterator != _cache._cache.end()) {
      ++_overall_hits;
      _monitor_key(key_tuple);

      return iterator->second.value;
    }

    // clang-format off
    auto value = _function(
          std::forward<Keys>(keys)...,
          std::forward<Rest>(rest)...
    );  // NOLINT(whitespace/parens)
    // clang-format on

    _cache.insert(key_tuple, value);

    return value;
  }

  auto hit_rate() const noexcept {
    return 1.0f * _overall_hits / _accesses;
  }

  auto cache_hits_for(const Key &key) const {
    if (_element_hits.count(key) == 0) {
      throw std::out_of_range{"Requested key not monitored."};
    }

    return _element_hits.at(key);
  }

 private:
  using HitMap = std::unordered_map<Key, size_t>;

  void _monitor_key(const Key &key) {
    auto element_iterator = _element_hits.find(key);
    if (element_iterator != _element_hits.end()) {
      ++(element_iterator->second);
    }
  }

  const Function &_function;
  LRU::Cache<Key, Value> _cache;
  HitMap _element_hits;

  size_t _overall_hits;
  size_t _accesses;
};


template <typename... Keys, typename Function>
static auto
memoize(const Function &function, size_t capacity, Keys &&... keys) {
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
memoize(const Function &function,
        size_t capacity = Internal::Globals::DEFAULT_CAPACITY,
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
