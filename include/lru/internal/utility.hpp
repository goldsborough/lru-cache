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

#ifndef LRU_UTILITY_HPP
#define LRU_UTILITY_HPP

#include <cstddef>
#include <iterator>
#include <tuple>
#include <utility>

namespace LRU {
namespace Internal {

template <typename... Ts>
constexpr auto tuple_indices(const std::tuple<Ts...>&) {
  return std::make_index_sequence<sizeof...(Ts)>();
}

template <typename T, typename... Args, std::size_t... Indices>
constexpr T construct_from_tuple(const std::tuple<Args...>& args,
                                 std::index_sequence<Indices...>) {
  return T(std::forward<Args>(std::get<Indices>(args))...);
}

template <typename T, typename... Args>
constexpr T construct_from_tuple(const std::tuple<Args...>& args) {
  return construct_from_tuple<T>(args, tuple_indices(args));
}

template <typename T, typename... Args>
constexpr T construct_from_tuple(Args&&... args) {
  return construct_from_tuple<T>(
      std::forward_as_tuple(std::forward<Args>(args)...));
}

template <typename T>
using enable_if_iterator = typename std::iterator_traits<T>::value_type;

template <typename T>
using enable_if_range = enable_if_iterator<decltype(std::declval<T>().begin())>;

template <typename T>
using enable_if_iterator_over_pair =
    typename std::iterator_traits<T>::value_type::first_type;

}  // namespace Internal
}  // namespace LRU

#endif  // LRU_UTILITY_HPP
