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

#ifndef LRU_INTERNAL_INFORMATION_HPP
#define LRU_INTERNAL_INFORMATION_HPP

#include <cstddef>
#include <tuple>
#include <utility>

#include "lru/internal/definitions.hpp"
#include "lru/internal/utility.hpp"

namespace LRU {
namespace Internal {

template <typename Key, typename Value>
struct Information {
  using KeyType = Key;
  using ValueType = Value;
  using QueueIterator = typename Internal::Queue<Key>::const_iterator;

  Information(const Value& value_, const QueueIterator& order_)
  : value(value_), order(order_) {
  }

  template <typename... ValueArguments>
  Information(const QueueIterator& order_, ValueArguments&&... value_argument)
  : value(std::forward<ValueArguments>(value_argument)...), order(order_) {
  }

  template <typename... ValueArguments>
  Information(const QueueIterator& order_,
              const std::tuple<ValueArguments...>& value_arguments)
  : Information(
        order_, value_arguments, Internal::tuple_indices(value_arguments)) {
  }

  Information(const Information& other) = default;
  Information(Information&& other) = default;

  virtual ~Information() = default;

  virtual bool operator==(const Information& other) const noexcept {
    if (this == &other) return true;
    if (this->value != other.value) return false;
    if (*this->order != *other.order) return false;
    return true;
  }

  virtual bool operator!=(const Information& other) const noexcept {
    return !(*this == other);
  }

  Value value;
  QueueIterator order;

 private:
  template <typename... ValueArguments, std::size_t... Indices>
  Information(const QueueIterator& order_,
              const std::tuple<ValueArguments...>& value_argument,
              std::index_sequence<Indices...>)
  : value(std::forward<ValueArguments>(std::get<Indices>(value_argument))...)
  , order(order_) {
  }
};

template <typename Key, typename Value>
struct TimedInformation : public Information<Key, Value> {
  using super = Information<Key, Value>;
  using typename super::QueueIterator;
  using Timestamp = Internal::Timestamp;

  TimedInformation(const Value& value_,
                   const QueueIterator& order_,
                   const Timestamp& insertion_time_)
  : super(value_, order_), insertion_time(insertion_time_) {
  }

  TimedInformation(const Value& value_, const QueueIterator& order_)
  : TimedInformation(value_, order_, Internal::Clock::now()) {
  }

  template <typename... ValueArguments>
  TimedInformation(const QueueIterator& order_,
                   ValueArguments&&... value_argument)
  : super(std::forward<ValueArguments>(value_argument)..., order_)
  , insertion_time(Internal::Clock::now()) {
  }

  template <typename... ValueArguments>
  TimedInformation(const QueueIterator& order_,
                   const std::tuple<ValueArguments...>& value_arguments)
  : super(order_, value_arguments), insertion_time(Internal::Clock::now()) {
  }

  bool operator==(const TimedInformation& other) const noexcept {
    if (super::operator!=(other)) return false;
    return this->insertion_time == other.insertion_time;
  }

  bool operator!=(const TimedInformation& other) const noexcept {
    return !(*this == other);
  }

  const Timestamp insertion_time;
};
}  // namespace Internal
}  // namespace LRU

#endif  // LRU_INTERNAL_INFORMATION_HPP
