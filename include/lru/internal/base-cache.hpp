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

#ifndef LRU_INTERNAL_BASE_CACHE_HPP
#define LRU_INTERNAL_BASE_CACHE_HPP

#include <cstddef>
#include <list>
#include <unordered_map>

#include "lru/internal/base-ordered-iterator.hpp"
#include "lru/internal/base-unordered-iterator.hpp"
#include "lru/internal/definitions.hpp"
#include "lru/internal/last-accessed.hpp"
#include "lru/internal/optional.hpp"
#include "lru/pair.hpp"

namespace LRU {
namespace Internal {

// Macros are bad, but also more readable sometimes:
// Without this macro, it becomes a pain to have a `using` directive for every
// new member we add to the `BaseCache` and rename or remove every such
// directive when we make a change to the `BaseCache`.
// With this macro, you can simply do:
// using super = BaseCache<Key, Value, Information>;
// using CACHE_BASE_MEMBERS;
#define CACHE_BASE_MEMBERS                \
  super::_cache;                          \
  using super::_order;                    \
  using super::_last_accessed;            \
  using super::_capacity;                 \
  using super::is_full;                   \
  using super::_erase;                    \
  using super::_erase_lru;                \
  using super::is_empty;                  \
  using super::_move_to_front;            \
  using super::_value_from_result;        \
  using typename super::Map;              \
  using typename super::MapIterator;      \
  using typename super::MapConstIterator; \
  using typename super::Queue;            \
  using typename super::QueueIterator;    \
  using typename super::Information;

template <typename Key,
          typename Value,
          template <typename, typename> class InformationType>
class BaseCache {
 public:
  using Information = InformationType<Key, Value>;

 protected:
  using Queue = Internal::Queue<Key>;
  using QueueIterator = typename Queue::const_iterator;

  using Map = Internal::Map<Key, Information>;
  using MapIterator = typename Map::iterator;
  using MapConstIterator = typename Map::const_iterator;

 public:
  class UnorderedIterator : public BaseUnorderedIterator<MapIterator> {
   private:
    using super = BaseUnorderedIterator<MapIterator>;
    friend class BaseCache;

    UnorderedIterator(MapIterator iterator)  // NOLINT(runtime/explicit)
        : super(iterator) {
    }
  };

  class UnorderedConstIterator
      : public BaseUnorderedIterator<MapConstIterator> {
   private:
    using super = BaseUnorderedIterator<MapConstIterator>;
    friend class BaseCache;

    UnorderedConstIterator(
        MapConstIterator iterator)  // NOLINT(runtime/explicit)
        : super(iterator) {
    }
  };

  class OrderedIterator : public BaseOrderedIterator<Key, Value, BaseCache> {
   private:
    using super = BaseOrderedIterator<Key, Value, BaseCache>;
    using UnderlyingIterator = typename super::UnderlyingIterator;
    friend class BaseCache;

    OrderedIterator(BaseCache& cache, UnderlyingIterator iterator)
    : super(cache, iterator) {
    }
  };

  class OrderedConstIterator
      : public BaseOrderedIterator<Key, const Value, const BaseCache> {
   private:
    using super = BaseOrderedIterator<Key, const Value, const BaseCache>;
    using UnderlyingIterator = typename super::UnderlyingIterator;
    friend class BaseCache;

    OrderedConstIterator(const BaseCache& cache, UnderlyingIterator iterator)
    : super(cache, iterator) {
    }
  };

  using size_t = std::size_t;

  explicit BaseCache(size_t capacity) : _capacity(capacity) {
  }

  virtual ~BaseCache() = default;

  UnorderedIterator unordered_begin() {
    return _cache.begin();
  }

  UnorderedConstIterator unordered_begin() const {
    return unordered_cbegin();
  }

  UnorderedConstIterator unordered_cbegin() const {
    return _cache.cbegin();
  }

  UnorderedIterator unordered_end() {
    return _cache.end();
  }

  UnorderedConstIterator unordered_end() const {
    return unordered_cend();
  }

  UnorderedConstIterator unordered_cend() const {
    return _cache.cend();
  }

  OrderedIterator ordered_begin() {
    return {*this, _order.begin()};
  }

  OrderedConstIterator ordered_begin() const {
    return ordered_cbegin();
  }

  OrderedConstIterator ordered_cbegin() const {
    return {*this, _order.cbegin()};
  }

  OrderedIterator ordered_end() {
    return {*this, _order.end()};
  }

  OrderedConstIterator ordered_end() const {
    return ordered_cend();
  }

  OrderedConstIterator ordered_cend() const {
    return {*this, _order.cend()};
  }

  OrderedIterator begin() {
    return ordered_begin();
  }

  OrderedConstIterator begin() const {
    return cbegin();
  }

  OrderedConstIterator cbegin() const {
    return ordered_begin();
  }

  OrderedIterator end() {
    return ordered_end();
  }

  OrderedConstIterator end() const {
    return cend();
  }

  OrderedConstIterator cend() const {
    return ordered_cend();
  }


  virtual bool contains(const Key& key) const = 0;


  // The non const verions are tricky because the user may change the
  // value but we will *not* insert it at the back! only insert() does that
  virtual Value& lookup(const Key& key) = 0;

  virtual const Value& lookup(const Key& key) const = 0;

  virtual Value& operator[](const Key& key) {
    return lookup(key);
  }

  virtual const Value& operator[](const Key& key) const {
    return lookup(key);
  }

  virtual Value& insert(const Key& key, const Value& value) = 0;

  virtual void erase(const Key& key) {
    if (_last_accessed == key) {
      _erase(_last_accessed.iterator());
    }

    auto iterator = _cache.find(key);
    if (iterator != _cache.end()) {
      _erase(iterator);
    }
  }

  virtual void clear() {
    _cache.clear();
    _order.clear();
  }

  size_t size() const noexcept {
    return _cache.size();
  }

  void capacity(size_t new_capacity) {
    // Pop the front of the cache if we have to resize
    while (size() > new_capacity) {
      _erase_lru();
    }
    _capacity = new_capacity;
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

 protected:
  using MapInsertionResult = decltype(Map().emplace());

  using LastAccessed = typename Internal::LastAccessed<MapConstIterator>;

  virtual void _move_to_front(MapIterator iterator, const Value& new_value) {
    _order.erase(iterator->second.order);

    // Insert and get the iterator (push_back returns
    // void and emplace_back returns a reference ...)
    auto new_order = _order.insert(_order.end(), iterator->first);

    iterator->second.order = new_order;
    iterator->second.value = new_value;

    _last_accessed = iterator;
  }

  virtual void _erase_lru() {
    _erase(_cache.find(_order.front()));
  }

  virtual void _erase(MapConstIterator iterator) {
    if (_last_accessed == iterator) {
      _last_accessed.invalidate();
    }

    _order.erase(iterator->second.order);
    _cache.erase(iterator);
  }

  virtual Value& _value_from_result(MapInsertionResult& result) {
    // `result.first` is the map iterator (to a pair), whose `second` member is
    // the information object, whose `value` member is the value stored.
    return result.first->second.value;
  }

  Map _cache;
  Queue _order;

  mutable LastAccessed _last_accessed;

  size_t _capacity;
};
}
}

#endif /* LRU_INTERNAL_BASE_CACHE_HPP*/
