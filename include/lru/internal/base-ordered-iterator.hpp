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

#ifndef BASE_ORDERED_ITERATOR_HPP
#define BASE_ORDERED_ITERATOR_HPP

#include <algorithm>
#include <functional>
#include <iterator>
#include <type_traits>

#include "lru/error.hpp"
#include "lru/internal/base-iterator.hpp"
#include "lru/internal/base-unordered-iterator.hpp"
#include "lru/internal/definitions.hpp"
#include "lru/internal/optional.hpp"
#include "lru/pair.hpp"

namespace LRU {
namespace Internal {

template <typename Key, typename Value, typename Cache>
using BaseForBaseOrderedIterator =
    BaseIterator<std::bidirectional_iterator_tag,
                 Key,
                 Value,
                 Cache,
                 typename Queue<Key>::const_iterator>;

template <typename Key, typename Value, typename Cache>
class BaseOrderedIterator
    : public BaseForBaseOrderedIterator<Key, Value, Cache> {
 protected:
  using super = BaseForBaseOrderedIterator<Key, Value, Cache>;
  using PRIVATE_BASE_ITERATOR_MEMBERS;
  using UnderlyingIterator = typename Queue<Key>::const_iterator;

 public:
  using Tag = std::true_type;
  using PUBLIC_BASE_ITERATOR_MEMBERS;

  BaseOrderedIterator() noexcept = default;

  BaseOrderedIterator(Cache& cache, UnderlyingIterator iterator)
  : super(cache, iterator) {
  }

  template <typename AnyKey, typename AnyValue, typename AnyCache>
  BaseOrderedIterator(
      const BaseOrderedIterator<AnyKey, AnyValue, AnyCache>& other)
  : super(other) {
  }

  template <typename AnyKey, typename AnyValue, typename AnyCache>
  BaseOrderedIterator(BaseOrderedIterator<AnyKey, AnyValue, AnyCache>&& other)
  : super(std::move(other)) {
  }

  template <
      typename AnyCache,
      typename UnderlyingIterator,
      typename = std::enable_if_t<
          std::is_same<std::decay_t<AnyCache>, std::decay_t<Cache>>::value>>
  BaseOrderedIterator(const BaseUnorderedIterator<AnyCache, UnderlyingIterator>&
                          unordered_iterator) {
    // Atomicity
    _check_if_at_end(unordered_iterator);
    _cache = unordered_iterator._cache;
    _iterator = unordered_iterator._iterator->second.order;
  }

  template <
      typename AnyCache,
      typename UnderlyingIterator,
      typename = std::enable_if_t<
          std::is_same<std::decay_t<AnyCache>, std::decay_t<Cache>>::value>>
  BaseOrderedIterator(BaseUnorderedIterator<AnyCache, UnderlyingIterator>&&
                          unordered_iterator) {
    // Atomicity
    _check_if_at_end(unordered_iterator);
    _cache = std::move(unordered_iterator._cache);
    _pair = std::move(unordered_iterator._pair);
    _iterator = std::move(unordered_iterator._iterator->second.order);
  }

  // If one special member function is defined, all must be.
  BaseOrderedIterator(const BaseOrderedIterator& other) = default;
  BaseOrderedIterator& operator=(const BaseOrderedIterator& other) = default;
  BaseOrderedIterator(BaseOrderedIterator&& other) = default;

  virtual ~BaseOrderedIterator() = default;

  bool operator==(const BaseOrderedIterator& other) const noexcept {
    return this->_iterator == other._iterator;
  }

  bool operator!=(const BaseOrderedIterator& other) const noexcept {
    return !(*this == other);
  }

  template <typename AnyCache, typename AnyUnderlyingIterator>
  bool operator==(const BaseUnorderedIterator<AnyCache, AnyUnderlyingIterator>&
                      second) const noexcept {
    if (this->_cache != second._cache) return false;

    // The past-the-end iterators of the same cache should compare equal
    // This is an exceptional guarantee we make. This is also the reason
    // why we can't rely on the conversion from unordered to ordered iterators
    // because construction of an ordered iterator from the past-the-end
    // unordered iterator will fail (with an InvalidIteratorConversion error)
    if (second == second._cache->unordered_end()) {
      return *this == this->_cache->ordered_end();
    }

    // Will call the other overload
    return *this == static_cast<BaseOrderedIterator>(second);
  }

  template <typename AnyCache, typename AnyUnderlyingIterator>
  friend bool operator==(
      const BaseUnorderedIterator<AnyCache, AnyUnderlyingIterator>& first,
      const BaseOrderedIterator& second) noexcept {
    return second == first;
  }

  template <typename AnyCache, typename AnyUnderlyingIterator>
  friend bool
  operator!=(const BaseOrderedIterator& first,
             const BaseUnorderedIterator<AnyCache, AnyUnderlyingIterator>&
                 second) noexcept {
    return !(first == second);
  }

  template <typename AnyCache, typename AnyUnderlyingIterator>
  friend bool operator!=(
      const BaseUnorderedIterator<AnyCache, AnyUnderlyingIterator>& first,
      const BaseOrderedIterator& second) noexcept {
    return second != first;
  }

  BaseOrderedIterator& operator++() {
    ++_iterator;
    _pair.reset();
    return *this;
  }

  BaseOrderedIterator operator++(int) {
    auto previous = *this;
    ++*this;
    return previous;
  }

  BaseOrderedIterator& operator--() {
    --_iterator;
    _pair.reset();
    return *this;
  }

  BaseOrderedIterator operator--(int) {
    auto previous = *this;
    --*this;
    return previous;
  }

  Pair& pair() noexcept override {
    return _maybe_lookup();
  }

  Value& value() noexcept override {
    return _maybe_lookup().value();
  }

  const Key& key() noexcept override {
    return *_iterator;
  }

 protected:
  template <typename, typename, typename>
  friend class BaseOrderedIterator;

  Pair& _maybe_lookup() {
    if (!_pair.has_value()) {
      _lookup();
    }

    return *_pair;
  }

  void _lookup() {
    Value& value = _cache->lookup(key());
    _pair.emplace(key(), value);
  }

 private:
  template <typename UnorderedIterator>
  void _check_if_at_end(const UnorderedIterator& unordered_iterator) {
    // For atomicity of the copy assignment, we assign the cache pointer only
    // after this check in the copy/move constructor and use the iterator's
    // cache. If an exception is thrown, the state of the ordered iterator is
    // unchanged compared to before the assignment.
    if (unordered_iterator == unordered_iterator._cache->unordered_end()) {
      throw LRU::Error::InvalidIteratorConversion();
    }
  }
};

}  // namespace Internal
}  // namespace LRU

#endif  // BASE_ORDERED_ITERATOR_HPP
