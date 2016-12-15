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

#ifndef LRU_INTERNAL_INFORMATION_HPP
#define LRU_INTERNAL_INFORMATION_HPP

#include <utility>

#include "lru/internal/globals.hpp"

namespace LRU {
namespace Internal {

template <typename Key, typename Value>
struct Information {
  using QueueIterator = typename Internal::QueueIterator<Key>;
  using Arguments = std::pair<const Value &, QueueIterator>;

  Information(const Value &value_, QueueIterator order_)
  : value(value_), order(order_) {
  }

  Information(const Arguments &arguments)
  : Information(arguments.first, arguments.second) {
  }

  const Value value;
  QueueIterator order;
};

template <typename Key, typename Value>
struct TimedInformation : public Information<Key, Value> {
  using super = Information<Key, Value>;
  using QueueIterator = typename super::QueueIterator;
  using Arguments = typename super::Arguments;

  TimedInformation(const Value &value_, QueueIterator order_)
  : super(value_, order_) {
  }

  TimedInformation(const Arguments &arguments) : super(arguments) {
  }

  const Internal::Timestamp insertion_time;
};
}
}

#endif /* LRU_INTERNAL_INFORMATION_HPP*/
