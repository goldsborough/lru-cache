#include <iostream>

#include "lru/lru.hpp"

int fibonacci(int n) {
  if (n < 2) return 1;
  return fibonacci(n - 1) + fibonacci(n - 2);
}

auto main() -> int {
  auto fib = LRU::memoize<int>(fibonacci);
  std::cout << fib(10) << std::endl;
  std::cout << fib.hit_rate() << std::endl;
  std::cout << fib.total_accesses() << std::endl;
  std::cout << fib.total_hits() << std::endl;

  std::cout << fib(10) << std::endl;
  std::cout << fib.hit_rate() << std::endl;
}
