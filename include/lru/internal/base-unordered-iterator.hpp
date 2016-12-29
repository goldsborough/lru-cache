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

#ifndef BASE_UNORDERED_ITERATOR_HPP
#define BASE_UNORDERED_ITERATOR_HPP

#include <iterator>
#include <type_traits>

#include "lru/internal/definitions.hpp"
#include "lru/internal/optional.hpp"
#include "lru/pair.hpp"


namespace LRU {
namespace Internal {

template <typename Cache, typename UnderlyingIterator>
class BaseUnorderedIterator
    : public std::iterator<
          std::forward_iterator_tag,
          LRU::Internal::Pair<decltype(UnderlyingIterator()->first),
                              decltype(UnderlyingIterator()->second)>> {
 public:
  using Tag = std::false_type;
  using Key = decltype(UnderlyingIterator()->first);
  using Value =
      std::conditional_t<std::is_const<Cache>::value,
                         const decltype(UnderlyingIterator()->second.value),
                         decltype(UnderlyingIterator()->second.value)>;
  using Pair = LRU::Internal::Pair<Key, Value>;

  BaseUnorderedIterator() = default;

  explicit BaseUnorderedIterator(Cache& cache, UnderlyingIterator iterator)
  : _iterator(iterator), _cache(cache) {
  }


  template <typename AnyCache, typename AnyIterator>
  bool
  operator==(const BaseUnorderedIterator<AnyCache, AnyIterator>& other) const
      noexcept {
    return this->_iterator == other._iterator;
  }

  template <typename AnyCache, typename AnyIterator>
  bool
  operator!=(const BaseUnorderedIterator<AnyCache, AnyIterator>& other) const
      noexcept {
    return !(*this == other);
  }

  BaseUnorderedIterator& operator++() {
    ++_iterator;
    _pair.reset();
    return *this;
  }

  BaseUnorderedIterator operator++(int) {
    auto previous = *this;
    ++*this;
    return previous;
  }

  Pair& operator*() noexcept {
    return pair();
  }

  Pair* operator->() noexcept {
    return &(**this);
  }

  Pair& pair() noexcept {
    if (!_pair.has_value()) {
      _pair.emplace(key(), value());
    }

    return *_pair;
  }

  Value& value() noexcept {
    return _iterator->second.value;
  }

  const Key& key() noexcept {
    return _iterator->first;
  }

 protected:
  template <typename, typename, typename>
  friend class BaseOrderedIterator;

  UnderlyingIterator _iterator;
  Optional<Pair> _pair;
  Cache& _cache;
};
}  // namespace Internal
}  // namespace LRU

#endif  // BASE_UNORDERED_ITERATOR_HPP
