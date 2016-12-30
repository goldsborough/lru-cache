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

#ifndef LRU_PAIR_HPP
#define LRU_PAIR_HPP

#include <algorithm>
#include <type_traits>
#include <utility>

namespace LRU {
namespace Internal {
template <typename Key, typename Value>
struct Pair final {
  Pair(const Key& key, Value& value) : first(key), second(value) {
  }

  template <typename AnyKey,
            typename AnyValue,
            typename =
                std::enable_if_t<(std::is_convertible<AnyKey, Key>::value &&
                                  std::is_convertible<AnyValue, Value>::value)>>
  Pair(const Pair<AnyKey, AnyValue>& other)
  : first(other.first), second(other.second) {
  }

  const Key& key() const noexcept {
    return first;
  }

  Value& value() noexcept {
    return second;
  }

  const Value& value() const noexcept {
    return second;
  }

  const Key& first;
  Value& second;
};
}  // namespace Internal
}  // namespace LRU


#endif  // LRU_PAIR_HPP
