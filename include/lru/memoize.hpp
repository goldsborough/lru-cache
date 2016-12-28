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

#ifndef LRU_MEMOIZE_HPP
#define LRU_MEMOIZE_HPP

#include <cstddef>
#include <tuple>
#include <unordered_map>
#include <utility>

#include "lru/cache.hpp"
#include "lru/errors.hpp"
#include "lru/internal/definitions.hpp"
#include "lru/internal/hash.hpp"
#include "lru/statistics.hpp"

namespace LRU {

template <typename Function>
auto wrap(const Function& original_function) {
  return [original_function](auto&&... arguments) {
    using Arguments = std::tuple<std::decay_t<decltype(arguments)>...>;
    using ReturnType = decltype(
        original_function(std::forward<decltype(arguments)>(arguments)...));

    static std::unordered_map<Arguments, ReturnType> cache;

    auto key = std::make_tuple(arguments...);
    auto iterator = cache.find(key);

    if (iterator != cache.end()) {
      return iterator->second;
    }

    auto value =
        original_function(std::forward<decltype(arguments)...>(arguments)...);
    cache.emplace(key, value);

    return value;
  };
}
}

#endif /* LRU_MEMOIZE_HPP*/
