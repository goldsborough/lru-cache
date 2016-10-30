#ifndef LRU_INTERNAL_HASH_HPP
#define LRU_INTERNAL_HASH_HPP

#include <cstddef>
#include <functional>
#include <tuple>

/**
 * `std::hash` specialization to allow storing tuples as keys
 * in `std::unordered_map`.
 *
 * Essentially hashes all tuple elements and jumbles the
 * individual hashes together.
 */
namespace std {
template <typename... Ts>
struct hash<std::tuple<Ts...>> {
  using argument_type = std::tuple<Ts...>;
  using result_type = std::size_t;
  static constexpr auto argument_size = std::tuple_size<argument_type>::value;

  result_type operator()(const argument_type &argument) const {
    return hash_tuple(argument, std::make_index_sequence<argument_size>());
  }

  template <std::size_t I, std::size_t... Is>
  result_type
  hash_tuple(const argument_type &t, std::index_sequence<I, Is...>) const {
    auto value = std::get<I>(t);
    auto result = std::hash<decltype(value)>{}(value);
    return result ^ hash_tuple(t, std::index_sequence<Is...>()) << 2;
  }

  result_type hash_tuple(const argument_type &t, std::index_sequence<>) const {
    return 0;
  }
};
}

#endif /* LRU_INTERNAL_HASH_HPP*/
