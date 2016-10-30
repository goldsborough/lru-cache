#include <iostream>

#include "lru/lru.hpp"

int fibonacci(int n) {
  if (n < 2) return 1;
  return fibonacci(n - 1) + fibonacci(n - 2);
}

auto main() -> int {
  auto fib = LRU::memoize<int>(fibonacci, 1, 2);
  std::cout << fib(5) << std::endl;
  std::cout << fib.hit_rate() << std::endl;
  std::cout << fib.cache_hits_for(10) << std::endl;

  std::cout << fib(10) << std::endl;
  std::cout << fib.hit_rate() << std::endl;
  std::cout << fib.cache_hits_for(10) << std::endl;
}
