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

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include "gtest/gtest.h"

#include "lru/lru.hpp"

using namespace LRU;

struct CacheTest : public ::testing::Test {
  template <typename Cache, typename Range>
  bool is_equal_to_range(const Cache& cache, const Range& range) {
    using std::begin;
    return std::equal(cache.ordered_begin(), cache.ordered_end(), begin(range));
  }

  Cache<std::string, int> cache;
};

TEST(CacheConstructionTest, IsConstructibleFromInitializerList) {
  Cache<std::string, int> cache = {
      {"one", 1}, {"two", 2}, {"three", 3},
  };

  EXPECT_FALSE(cache.is_empty());
  EXPECT_EQ(cache.size(), 3);
  EXPECT_EQ(cache["one"], 1);
  EXPECT_EQ(cache["two"], 2);
  EXPECT_EQ(cache["three"], 3);
}

TEST(CacheConstructionTest, IsConstructibleFromInitializerListWithCapacity) {
  // clang-format off
  Cache<std::string, int> cache(2, {
    {"one", 1}, {"two", 2}, {"three", 3},
  });
  // clang-format on

  EXPECT_FALSE(cache.is_empty());
  EXPECT_EQ(cache.size(), 2);
  EXPECT_FALSE(cache.contains("one"));
  EXPECT_EQ(cache["two"], 2);
  EXPECT_EQ(cache["three"], 3);
}

TEST(CacheConstructionTest, IsConstructibleFromRange) {
  std::vector<std::pair<std::string, int>> range = {
      {"one", 1}, {"two", 2}, {"three", 3}};

  Cache<std::string, int> cache(range);

  EXPECT_FALSE(cache.is_empty());
  EXPECT_EQ(cache.size(), 3);
  EXPECT_EQ(cache["one"], 1);
  EXPECT_EQ(cache["two"], 2);
  EXPECT_EQ(cache["three"], 3);
}

TEST(CacheConstructionTest, IsConstructibleFromIterators) {
  std::vector<std::pair<std::string, int>> range = {
      {"one", 1}, {"two", 2}, {"three", 3}};

  Cache<std::string, int> cache(range.begin(), range.end());

  EXPECT_FALSE(cache.is_empty());
  EXPECT_EQ(cache.size(), 3);
  EXPECT_EQ(cache["one"], 1);
  EXPECT_EQ(cache["two"], 2);
  EXPECT_EQ(cache["three"], 3);
}

TEST(CacheConstructionTest, UsesCustomHashFunction) {
  std::size_t mock_hash_call_count = 0;
  const auto mock_hash = [&mock_hash_call_count](int value) {
    mock_hash_call_count += 1;
    return value;
  };

  Cache<int, int, decltype(mock_hash)> cache(128, mock_hash);

  EXPECT_EQ(mock_hash_call_count, 0);

  cache.contains(5);
  EXPECT_EQ(mock_hash_call_count, 1);
}

TEST(CacheConstructionTest, UsesCustomKeyEqual) {
  std::size_t mock_equal_call_count = 0;
  const auto mock_equal = [&mock_equal_call_count](int a, int b) {
    mock_equal_call_count += 1;
    return a == b;
  };

  Cache<int, int, std::hash<int>, decltype(mock_equal)> cache(
      128, std::hash<int>(), mock_equal);

  EXPECT_EQ(mock_equal_call_count, 0);

  cache.insert(5, 1);
  ASSERT_TRUE(cache.contains(5));
  EXPECT_EQ(mock_equal_call_count, 1);
}

TEST_F(CacheTest, CanInsertIterators) {
  std::vector<std::pair<std::string, int>> range = {
      {"one", 1}, {"two", 2}, {"three", 3}};

  cache.insert(range.begin(), range.end());
  EXPECT_TRUE(is_equal_to_range(cache, range));
}

TEST_F(CacheTest, CanInsertRange) {
  std::vector<std::pair<std::string, int>> range = {
      {"one", 1}, {"two", 2}, {"three", 3}};

  cache.insert(range);
  EXPECT_TRUE(is_equal_to_range(cache, range));
}

TEST_F(CacheTest, CanInsertList) {
  std::initializer_list<std::pair<std::string, int>> list = {
      {"one", 1}, {"two", 2}, {"three", 3}};

  // Do it like this, just to verify that template deduction fails if only
  // the range function exists and no explicit overload for the initializer list
  cache.insert({{"one", 1}, {"two", 2}, {"three", 3}});
  EXPECT_TRUE(is_equal_to_range(cache, list));
}
