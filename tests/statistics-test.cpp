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

#include <vector>

#include "gtest/gtest.h"

#include "lru/error.hpp"
#include "lru/internal/statistics-mutator.hpp"
#include "lru/lru.hpp"

using namespace LRU;
using namespace LRU::Internal;

TEST(StatisticsTest, ConstructsWellFromRange) {
  std::vector<int> range = {1, 2, 3};
  Statistics<int> stats(range);

  for (const auto& i : range) {
    ASSERT_TRUE(stats.is_monitoring(i));
  }
}

TEST(StatisticsTest, ConstructsWellFromIterator) {
  std::vector<int> range = {1, 2, 3};
  Statistics<int> stats(range.begin(), range.end());

  for (const auto& i : range) {
    ASSERT_TRUE(stats.is_monitoring(i));
  }
}

TEST(StatisticsTest, ConstructsWellFromInitializerList) {
  Statistics<int> stats({1, 2, 3});

  std::vector<int> range = {1, 2, 3};
  for (const auto& i : range) {
    ASSERT_TRUE(stats.is_monitoring(i));
  }
}

TEST(StatisticsTest, ConstructsWellFromVariadicArguments) {
  Statistics<int> stats(1, 2, 3);

  std::vector<int> range = {1, 2, 3};
  for (const auto& i : range) {
    ASSERT_TRUE(stats.is_monitoring(i));
  }
}

TEST(StatisticsTest, EmptyPreconditions) {
  Statistics<int> stats;

  EXPECT_FALSE(stats.is_monitoring_keys());
  EXPECT_EQ(stats.number_of_monitored_keys(), 0);
  EXPECT_FALSE(stats.is_monitoring(1));
  EXPECT_FALSE(stats.is_monitoring(2));
  EXPECT_EQ(stats.total_accesses(), 0);
  EXPECT_EQ(stats.total_hits(), 0);
  EXPECT_EQ(stats.total_misses(), 0);
}

TEST(StatisticsTest, StatisticsMutatorCanRegisterHits) {
  Statistics<int> stats(1);

  StatisticsMutator<int> mutator(stats);

  mutator.register_hit(1);
  EXPECT_EQ(stats.hits_for(1), 1);
  EXPECT_EQ(stats.total_accesses(), 1);
  EXPECT_EQ(stats.total_hits(), 1);
  EXPECT_EQ(stats.total_misses(), 0);
  EXPECT_EQ(stats.hit_rate(), 1);
  EXPECT_EQ(stats.miss_rate(), 0);

  mutator.register_hit(1);
  EXPECT_EQ(stats.hits_for(1), 2);
  EXPECT_EQ(stats.total_accesses(), 2);
  EXPECT_EQ(stats.total_hits(), 2);
  EXPECT_EQ(stats.total_misses(), 0);
  EXPECT_EQ(stats.hit_rate(), 1);
  EXPECT_EQ(stats.miss_rate(), 0);

  mutator.register_hit(2);
  EXPECT_EQ(stats.hits_for(1), 2);
  EXPECT_EQ(stats.hits_for(2), 1);
  EXPECT_EQ(stats.total_accesses(), 3);
  EXPECT_EQ(stats.total_hits(), 3);
  EXPECT_EQ(stats.total_misses(), 0);
  EXPECT_EQ(stats.hit_rate(), 1);
  EXPECT_EQ(stats.miss_rate(), 0);
}

TEST(StatisticsTest, StatisticsMutatorCanRegisterMisses) {
  Statistics<int> stats(1);

  StatisticsMutator<int> mutator(stats);

  mutator.register_miss(1);
  EXPECT_EQ(stats.misses_for(1), 1);
  EXPECT_EQ(stats.total_accesses(), 1);
  EXPECT_EQ(stats.total_hits(), 0);
  EXPECT_EQ(stats.total_misses(), 1);
  EXPECT_EQ(stats.hit_rate(), 0);
  EXPECT_EQ(stats.miss_rate(), 1);

  mutator.register_miss(1);
  EXPECT_EQ(stats.misses_for(1), 2);
  EXPECT_EQ(stats.total_accesses(), 2);
  EXPECT_EQ(stats.total_hits(), 0);
  EXPECT_EQ(stats.total_misses(), 2);
  EXPECT_EQ(stats.hit_rate(), 0);
  EXPECT_EQ(stats.miss_rate(), 1);

  mutator.register_miss(2);
  EXPECT_EQ(stats.misses_for(1), 2);
  EXPECT_EQ(stats.misses_for(2), 1);
  EXPECT_EQ(stats.total_accesses(), 3);
  EXPECT_EQ(stats.total_hits(), 0);
  EXPECT_EQ(stats.total_misses(), 3);
  EXPECT_EQ(stats.hit_rate(), 0);
  EXPECT_EQ(stats.miss_rate(), 1);
}

TEST(StatisticsTest, CanDynamicallyMonitorAndUnmonitorKeys) {
  Statistics<int> stats;

  ASSERT_EQ(stats.number_of_monitored_keys(), 0);

  stats.monitor(1);

  EXPECT_EQ(stats.number_of_monitored_keys(), 1);
  EXPECT_TRUE(stats.is_monitoring(1));
  EXPECT_FALSE(stats.is_monitoring(2));

  stats.monitor(2);

  EXPECT_EQ(stats.number_of_monitored_keys(), 2);
  EXPECT_TRUE(stats.is_monitoring(1));
  EXPECT_TRUE(stats.is_monitoring(2));

  stats.unmonitor(1);

  EXPECT_EQ(stats.number_of_monitored_keys(), 1);
  EXPECT_FALSE(stats.is_monitoring(1));
  EXPECT_TRUE(stats.is_monitoring(2));

  stats.unmonitor_all();

  EXPECT_FALSE(stats.is_monitoring_keys());
  EXPECT_FALSE(stats.is_monitoring(1));
  EXPECT_FALSE(stats.is_monitoring(2));
}

TEST(StatisticsTest, ThrowsForUnmonitoredKey) {
  Statistics<int> stats;

  EXPECT_THROW(stats.stats_for(1), LRU::Error::UnmonitoredKey);
  EXPECT_THROW(stats.hits_for(2), LRU::Error::UnmonitoredKey);
  EXPECT_THROW(stats.misses_for(3), LRU::Error::UnmonitoredKey);
  EXPECT_THROW(stats[4], LRU::Error::UnmonitoredKey);
}

TEST(StatisticsTest, RatesAreCalculatedCorrectly) {
  Statistics<int> stats(1, 2, 3);
  StatisticsMutator<int> mutator(stats);

  for (std::size_t i = 0; i < 20; ++i) {
    mutator.register_hit(1);
  }

  for (std::size_t i = 0; i < 80; ++i) {
    mutator.register_miss(1);
  }

  EXPECT_EQ(stats.hit_rate(), 0.2);
  EXPECT_EQ(stats.miss_rate(), 0.8);
}

TEST(StatisticsTest, CanShareStatistics) {
  Statistics<int> stats(1, 2, 3);
  StatisticsMutator<int> mutator1(stats);
  StatisticsMutator<int> mutator2(stats);
  StatisticsMutator<int> mutator3(stats);

  mutator1.register_hit(1);
  EXPECT_EQ(stats.total_accesses(), 1);
  EXPECT_EQ(stats.total_hits(), 1);
  EXPECT_EQ(stats.total_misses(), 0);
  EXPECT_EQ(stats.hits_for(1), 1);

  mutator2.register_hit(1);
  EXPECT_EQ(stats.total_accesses(), 2);
  EXPECT_EQ(stats.total_hits(), 2);
  EXPECT_EQ(stats.total_misses(), 0);
  EXPECT_EQ(stats.hits_for(1), 2);

  mutator3.register_miss(2);
  EXPECT_EQ(stats.total_accesses(), 3);
  EXPECT_EQ(stats.total_hits(), 2);
  EXPECT_EQ(stats.total_misses(), 1);
  EXPECT_EQ(stats.hits_for(1), 2);
  EXPECT_EQ(stats.misses_for(1), 0);
  EXPECT_EQ(stats.hits_for(2), 0);
  EXPECT_EQ(stats.misses_for(2), 1);
}
