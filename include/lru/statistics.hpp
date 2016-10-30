#ifndef LRU_STATISTICS_HPP
#define LRU_STATISTICS_HPP

#include <cstddef>

#include "lru/internal/globals.hpp"

namespace LRU {
template <typename HitMap>
struct Statistics {
  using size_t = std::size_t;

  double hit_rate;
  const HitMap& hits_for;

  size_t total_accesses;
  size_t total_hits;
  size_t total_misses;
};
}

#endif /* LRU_STATISTICS_HPP*/
