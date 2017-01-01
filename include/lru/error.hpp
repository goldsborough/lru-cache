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

#ifndef LRU_INTERNAL_ERRORS_HPP
#define LRU_INTERNAL_ERRORS_HPP

#include <stdexcept>
#include <string>

namespace LRU {
namespace Error {

struct KeyNotFound : public std::runtime_error {
  using super = std::runtime_error;

  KeyNotFound() : super("Failed to find key") {
  }

  explicit KeyNotFound(const std::string& key)
  : super("Failed to find key: " + key) {
  }
};

struct KeyExpired : public std::runtime_error {
  using super = std::runtime_error;

  explicit KeyExpired(const std::string& key)
  : super("Key found, but expired: " + key) {
  }

  KeyExpired() : super("Key found, but expired") {
  }
};

struct EmptyCache : public std::runtime_error {
  using super = std::runtime_error;
  explicit EmptyCache(const std::string& what_was_expected)
  : super("Requested " + what_was_expected + " of empty cache") {
  }
};

struct InvalidIteratorConversion : public std::runtime_error {
  using super = std::runtime_error;
  InvalidIteratorConversion()
  : super("Cannot convert past-the-end unordered to ordered iterator") {
  }
};

struct InvalidIterator : public std::runtime_error {
  using super = std::runtime_error;
  InvalidIterator() : super("Past-the-end iterator is invalid here") {
  }
};

struct UnmonitoredKey : public std::runtime_error {
  using super = std::runtime_error;
  UnmonitoredKey() : super("Requested statistics for unmonitored key") {
  }
};

struct NotMonitoring : public std::runtime_error {
  using super = std::runtime_error;
  NotMonitoring() : super("Statistics monitoring not enabled for this cache") {
  }
};

}  // namespace Error
}  // namespace LRU

#endif  // LRU_INTERNAL_ERRORS_HPP
