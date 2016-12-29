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

#include "gtest/gtest.h"
#include "lru/lru.hpp"

using namespace LRU;

struct IteratorTest : public ::testing::Test {
  using CacheType = Cache<std::string, int>;
  CacheType cache;
};

TEST_F(IteratorTest, OrderedAndUnorderedAreComparable) {
  cache.emplace("one", 1);

  ASSERT_EQ(cache.begin(), cache.begin());

  EXPECT_EQ(cache.unordered_begin(), cache.ordered_begin());

  // We need to ensure symmetry!
  EXPECT_EQ(cache.ordered_begin(), cache.unordered_begin());

  // This is an exceptional property we expect
  EXPECT_EQ(cache.unordered_end(), cache.ordered_end());
  EXPECT_EQ(cache.ordered_end(), cache.unordered_end());

  // These assumptions should hold because there is only one element
  // so the unordered iterator will convert to an ordered iterator, then
  // compare equal because both point to the same single element.
  EXPECT_EQ(cache.ordered_begin(), cache.unordered_begin());
  EXPECT_EQ(cache.unordered_begin(), cache.ordered_begin());

  cache.emplace("two", 1);

  // But then the usual assumptions should hold
  EXPECT_NE(cache.ordered_begin(), cache.find("two"));
  EXPECT_NE(cache.find("two"), cache.ordered_begin());
}

TEST_F(IteratorTest, TestConversionFromUnorderedToOrdered) {
  cache.emplace("one", 1);
  cache.emplace("two", 2);
  cache.emplace("three", 3);

  typename CacheType::UnorderedIterator unordered = cache.find("one");

  ASSERT_EQ(unordered.key(), "one");
  ASSERT_EQ(unordered.value(), 1);

  typename CacheType::OrderedIterator ordered(unordered);

  EXPECT_EQ(ordered.key(), "one");
  EXPECT_EQ(ordered.value(), 1);

  // Once it's ordered, the ordering shold be maintained
  ++ordered;
  EXPECT_EQ(ordered.key(), "two");
  EXPECT_EQ(ordered.value(), 2);
}
