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

#ifndef BASE_ITERATOR_HPP
#define BASE_ITERATOR_HPP

#include <algorithm>
#include <iterator>

#include "lru/internal/optional.hpp"
#include "lru/pair.hpp"

#define PUBLIC_BASE_ITERATOR_MEMBERS \
  typename super::Key;               \
  using typename super::Value;       \
  using typename super::Pair;        \
  using typename super::Cache;

#define PRIVATE_BASE_ITERATOR_MEMBERS \
  typename super::UnderlyingIterator; \
  using super::_iterator;             \
  using super::_pair;                 \
  using super::_cache;


namespace LRU {
namespace Internal {

template <typename IteratorTag,
          typename KeyType,
          typename ValueType,
          typename CacheType,
          typename UnderlyingIteratorType>
class BaseIterator
    : public std::iterator<IteratorTag, LRU::Pair<KeyType, ValueType>> {
 public:
  using Key = KeyType;
  using Cache = CacheType;
  using UnderlyingIterator = UnderlyingIteratorType;
  using Value = std::
      conditional_t<std::is_const<Cache>::value, const ValueType, ValueType>;
  using Pair = LRU::Pair<Key, Value>;

  BaseIterator() noexcept : _cache(nullptr) {
  }

  BaseIterator(Cache& cache,
               const UnderlyingIterator& iterator,
               const Optional<Pair>& pair = Optional<Pair>())
  : _iterator(iterator), _pair(pair), _cache(&cache) {
  }

  template <typename AnyIteratorTag,
            typename AnyKeyType,
            typename AnyValueType,
            typename AnyCacheType,
            typename AnyUnderlyingIteratorType>
  BaseIterator(const BaseIterator<AnyIteratorTag,
                                  AnyKeyType,
                                  AnyValueType,
                                  AnyCacheType,
                                  AnyUnderlyingIteratorType>& other)
  : _iterator(other._iterator), _pair(other._pair), _cache(other._cache) {
  }

  template <typename AnyIteratorTag,
            typename AnyKeyType,
            typename AnyValueType,
            typename AnyCacheType,
            typename AnyUnderlyingIteratorType>
  BaseIterator(BaseIterator<AnyIteratorTag,
                            AnyKeyType,
                            AnyValueType,
                            AnyCacheType,
                            AnyUnderlyingIteratorType>&& other)
  : _iterator(std::move(other._iterator))
  , _pair(std::move(other._pair))
  , _cache(std::move(other._cache)) {
  }

  virtual void swap(BaseIterator& other) noexcept {
    // Enable ADL
    using std::swap;

    swap(_iterator, other._iterator);
    swap(_pair, other._pair);
    swap(_cache, other._cache);
  }

  friend void swap(BaseIterator& first, BaseIterator& second) noexcept {
    first.swap(second);
  }

  Pair& operator*() noexcept {
    return pair();
  }

  Pair* operator->() noexcept {
    return &(**this);
  }

  virtual Pair& pair() noexcept = 0;
  virtual Value& value() noexcept = 0;
  virtual const Key& key() noexcept = 0;

 protected:
  template <typename, typename, typename, typename, typename>
  friend class BaseIterator;

  UnderlyingIterator _iterator;
  Optional<Pair> _pair;

  // Pointer and not reference because it's cheap to copy
  Cache* _cache;
};
}  // namespace Internal
}  // namespace LRU

#endif  // BASE_ITERATOR_HPP
