#include <iostream>

#include "lru-cache.hpp"

int fibonacci(int n) {
  if (n < 2) return 1;
  return fibonacci(n - 1) + fibonacci(n - 2);
}

auto main() -> int {
  auto fib = LRUCache<int, int>::memoize(fibonacci, 100, 128);
  std::cout << fib(10) << std::endl;
  std::cout << fib.hit_rate() << std::endl;
}
