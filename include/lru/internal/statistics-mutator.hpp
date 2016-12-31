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

#ifndef LRU_STATISTICS_MUTATOR_HPP
#define LRU_STATISTICS_MUTATOR_HPP

#include <utility>

#include "lru/internal/generalized-pointer.hpp"
#include "lru/internal/optional.hpp"
#include "lru/statistics.hpp"

namespace LRU {
namespace Internal {

template <typename Key>
class StatisticsMutator {
 public:
  StatisticsMutator() noexcept = default;

  explicit StatisticsMutator(Statistics<Key>& statistics)
  : _statistics(statistics) {
  }

  explicit StatisticsMutator(Statistics<Key>&& statistics)
  : _statistics(std::move(statistics)) {
  }

  void register_hit(const Key& key) {
    _statistics->_number_of_accesses += 1;
    _statistics->_number_of_hits += 1;
    _statistics->_hit_map[key].hits += 1;
  }

  void register_miss(const Key& key) {
    _statistics->_number_of_accesses += 1;
    _statistics->_hit_map[key].misses += 1;
  }

  Statistics<Key>& statistics() noexcept {
    return *_statistics;
  }

  const Statistics<Key>& statistics() const noexcept {
    return *_statistics;
  }

 private:
  Internal::GeneralizedPointer<Statistics<Key>> _statistics;
};

}  // namespace Internal
}  // namespace LRU

#endif  // LRU_STATISTICS_MUTATOR_HPP
