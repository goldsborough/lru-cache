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
