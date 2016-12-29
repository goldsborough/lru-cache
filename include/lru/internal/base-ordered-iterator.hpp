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

#include <functional>

#include "lru/internal/base-unordered-iterator.hpp"
#include "lru/internal/definitions.hpp"
#include "lru/internal/optional.hpp"
#include "lru/pair.hpp"

namespace LRU {
namespace Internal {
template <typename Key, typename Value, typename Cache>
class BaseOrderedIterator {
 public:
  using UnderlyingIterator = typename Queue<Key>::const_iterator;
  using Pair = LRU::Internal::Pair<Key, Value>;

  explicit BaseOrderedIterator(Cache& cache, UnderlyingIterator iterator)
  : _iterator(iterator), _cache(cache) {
  }

  template <typename UnderlyingIterator>
  BaseOrderedIterator(
      const BaseUnorderedIterator<Cache, UnderlyingIterator>& unordered)
  : _iterator(unordered._iterator->second.order), _cache(unordered._cache) {
  }

  bool operator==(const BaseOrderedIterator& other) const noexcept {
    return this->_iterator == other._iterator;
  }

  bool operator!=(const BaseOrderedIterator& other) const noexcept {
    return !(*this == other);
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

  Pair& operator*() noexcept {
    return pair();
  }

  Pair* operator->() noexcept {
    return &(**this);
  }

  Pair& pair() noexcept {
    return _maybe_lookup();
  }

  Value& value() noexcept {
    return _maybe_lookup()->value();
  }

  const Key& key() noexcept {
    return *_iterator;
  }

 protected:
  Pair& _maybe_lookup() {
    if (!_pair.has_value()) {
      _lookup();
    }

    return *_pair;
  }

  void _lookup() {
    Value& value = _cache.lookup(key());
    _pair.emplace(key(), value);
  }

  UnderlyingIterator _iterator;
  Optional<Pair> _pair;
  Cache& _cache;
};

}  // namespace Internal
}  // namespace LRU

#endif  // BASE_ORDERED_ITERATOR_HPP
