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

#include <string>
#include <utility>
#include <vector>

#include "gtest/gtest.h"

#include "lru/lru.hpp"

using namespace LRU;

TEST(CacheTest, IsConstructibleFromInitializerList) {
  Cache<std::string, int> cache = {
      {"one", 1}, {"two", 2}, {"three", 3},
  };

  EXPECT_FALSE(cache.is_empty());
  EXPECT_EQ(cache.size(), 3);
  EXPECT_EQ(cache["one"], 1);
  EXPECT_EQ(cache["two"], 2);
  EXPECT_EQ(cache["three"], 3);
}

TEST(CacheTest, IsConstructibleFromInitializerListWithCapacity) {
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

TEST(CacheTest, IsConstructibleFromRange) {
  // clang-format off
  std::vector<std::pair<std::string, int>> range = {
      {"one", 1}, {"two", 2}, {"three", 3}
  };
  // clang-format on

  Cache<std::string, int> cache(range);

  EXPECT_FALSE(cache.is_empty());
  EXPECT_EQ(cache.size(), 3);
  EXPECT_EQ(cache["one"], 1);
  EXPECT_EQ(cache["two"], 2);
  EXPECT_EQ(cache["three"], 3);
}

TEST(CacheTest, IsConstructibleFromIterators) {
  // clang-format off
  std::vector<std::pair<std::string, int>> range = {
      {"one", 1}, {"two", 2}, {"three", 3}
  };
  // clang-format on

  Cache<std::string, int> cache(range.begin(), range.end());

  EXPECT_FALSE(cache.is_empty());
  EXPECT_EQ(cache.size(), 3);
  EXPECT_EQ(cache["one"], 1);
  EXPECT_EQ(cache["two"], 2);
  EXPECT_EQ(cache["three"], 3);
}
