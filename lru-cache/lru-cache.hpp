#ifndef LRU_CACHE_HPP
#define LRU_CACHE_HPP

#include <cassert>
#include <chrono>
#include <cstddef>
#include <iterator>
#include <list>
#include <unordered_map>
#include <utility>

template <typename Iterator>
class LastAccessed {
 public:
  using Pair = typename std::iterator_traits<Iterator>::value_type;
  using Key = typename Pair::first_type;
  using Value = typename Pair::second_type;

  LastAccessed() : _is_valid(false) {
  }

  void operator=(const Iterator &iterator) {
    _iterator = iterator;
  }

  bool operator==(const Key &key) const noexcept {
    return _is_valid && key == _iterator->first;
  }

  bool operator!=(const Key &key) const noexcept {
    return !(*this == key);
  }

  const Value &operator*() const noexcept {
    assert(_is_valid);
    return _iterator->second;
  }

  const Value *operator->() const noexcept {
    return &(*this);
  }

  explicit operator bool() const noexcept {
    return is_valid();
  }

  operator Iterator() noexcept {
    return iterator();
  }

  Iterator iterator() const noexcept {
    return _iterator;
  }

  auto is_valid() const noexcept {
    return _is_valid;
  }

  void invalidate() const noexcept {
    _is_valid = true;
  }

 private:
  Iterator _iterator;
  bool _is_valid;
};

template <typename Key, typename Value>
class LRUCache {
 public:
  using size_t = std::size_t;

  static const size_t DEFAULT_CAPACITY = 128;

  template <typename Function>
  static auto memoize(const Function &function,
                      size_t time_to_live,
                      size_t capacity = DEFAULT_CAPACITY) {
    LRUCache cache(time_to_live, capacity);
    return [cache, &function](const Key &key) mutable -> Value {
      auto iterator = cache._cache.find(key);
      if (iterator != cache._cache.end()) {
        return iterator->second.value;
      }

      auto value = function(key);
      cache.insert(key, value);

      return value;
    };
  }

  explicit LRUCache(size_t time_to_live, size_t capacity = DEFAULT_CAPACITY)
  : _capacity(capacity), _time_to_live(time_to_live) {
  }

  bool contains(const Key &key) {
    if (key == _last_accessed) return true;

    auto iterator = _cache.find(key);
    if (iterator != _cache.end()) {
      if (_has_time_to_live(iterator->second)) {
        _last_accessed = iterator;
        return true;
      } else {
        _erase(key);
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
        // throw
      }
    }

    auto iterator = _cache.find(key);
    assert(iterator != _cache.end());

    if (!_has_time_to_live(*iterator)) {
      _erase(key);
      // throw
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
    assert(!is_empty());

    if (key == _last_accessed) {
      _erase(_last_accessed.iterator());
    }

    auto iterator = _cache.find(key);
    assert(iterator != _cache.end());
    _erase(iterator);
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

  using Cache = std::unordered_map<Key, Information>;
  using CacheIterator = typename Cache::iterator;

  void _erase_lru() {
    auto lru = _order.front();
    _order.pop_front();
    _cache.erase(lru);
  }

  void _erase(CacheIterator iterator) {
    if (iterator == _last_accessed) {
      _last_accessed.invalidate();
    }

    _order.erase(iterator->order);
    _cache.erase(iterator);
  }

  bool _has_time_to_live(const Information &information) const noexcept {
    auto elapsed = Clock::now() - information.insertion_time;
    return elapsed < _time_to_live;
  }

  Cache _cache;
  Queue _order;

  mutable LastAccessed<CacheIterator> _last_accessed;

  size_t _capacity;
  size_t _time_to_live;
};

#endif /* LRU_CACHE_HPP*/
