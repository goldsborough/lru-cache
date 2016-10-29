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
    _is_valid = true;
  }
  
  friend bool operator==(const LastAccessed<Iterator>& iterator, const Key &key) noexcept {
      return iterator._is_valid && key == iterator._iterator->first;
  }
  
  friend bool operator==(const LastAccessed<Iterator>& iterator, const Iterator &other) noexcept {
    return iterator._is_valid && other->first == iterator._iterator->first;
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

  void invalidate() noexcept {
    _is_valid = false;
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

  
  template <typename Function, typename... MonitoredKeys>
  class FunctionObject {
  public:
    FunctionObject(const Function &function, size_t time_to_live, size_t capacity = DEFAULT_CAPACITY, MonitoredKeys&&... monitoredKeys)
    : _function {function}, _cache {time_to_live, capacity}, _overall_hits {0}, _accesses {0}, _element_hits {{monitoredKeys, 0}...}    
    {        
    }
      
    auto operator()(const Key &key) {
      ++_accesses;
      
      auto iterator = _cache._cache.find(key);
      if (iterator != _cache._cache.end()) {
        ++_overall_hits;
        
        auto stat_it = _element_hits.find(key);
        if (stat_it != _element_hits.end()) {
          ++(stat_it->second);
        }
        
        return iterator->second.value;
      }

      auto value = _function(key);
      _cache.insert(key, value);
     
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
    const Function& _function;
    LRUCache _cache;
    
    size_t _overall_hits;
    size_t _accesses;
    
    std::unordered_map<Key, size_t> _element_hits;
  };
  
  
  template <typename Function, typename... MonitoredKeys>
  static auto memoize(const Function &function,
                      size_t time_to_live,
                      size_t capacity = DEFAULT_CAPACITY, MonitoredKeys&&... monitoredKeys) {
    
    return FunctionObject<Function, MonitoredKeys...>{function, time_to_live, capacity, std::forward<MonitoredKeys>(monitoredKeys)...};
  }

  explicit LRUCache(size_t time_to_live, size_t capacity = DEFAULT_CAPACITY)
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
    if (_last_accessed == iterator) {
      _last_accessed.invalidate();
    }

    _order.erase(iterator->second.order);
    _cache.erase(iterator);
  }

  bool _has_time_to_live(const Information &information) const noexcept {
    auto elapsed = Clock::now() - information.insertion_time;
    return std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() < _time_to_live;
  }

  Cache _cache;
  Queue _order;

  mutable LastAccessed<CacheIterator> _last_accessed;

  size_t _capacity;
  size_t _time_to_live;
};

#endif /* LRU_CACHE_HPP*/
