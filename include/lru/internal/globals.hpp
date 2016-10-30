#ifndef LRU_INTERNAL_GLOBALS_HPP
#define LRU_INTERNAL_GLOBALS_HPP

#include <chrono>
#include <cstddef>
#include <list>
#include <tuple>

namespace LRU {
namespace Internal {
const std::size_t DEFAULT_CAPACITY = 128;

template <typename T>
using Queue = std::list<T>;

template <typename T>
using QueueIterator = typename Queue<T>::const_iterator;

using Clock = std::chrono::steady_clock;
using Timestamp = Clock::time_point;
}
}


#endif /* LRU_INTERNAL_GLOBALS_HPP*/
