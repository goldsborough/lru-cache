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
template <typename Key, typename Value>
struct Pair final {
  using KeyType = Key;
  using ValueType = Value;
  using first_type = Key;
  using second_type = Value;

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

  bool operator==(const Pair& other) const noexcept {
    return this->first == other.first && this->second == other.second;
  }

  template <typename First, typename Second>
  friend bool operator==(const Pair& first,
                         const std::pair<First, Second>& second) noexcept {
    return first.first == second.first && first.second == second.second;
  }

  template <typename First, typename Second>
  friend bool operator==(const std::pair<First, Second>& first,
                         const Pair& second) noexcept {
    return second == first;
  }

  template <typename T>
  friend bool operator!=(const Pair& first, const T& second) noexcept {
    return !(first == second);
  }

  template <typename T>
  friend bool operator!=(const T& first, const Pair& second) noexcept {
    return second != first;
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
}  // namespace LRU


#endif  // LRU_PAIR_HPP
