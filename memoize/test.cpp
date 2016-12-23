#include <cstddef>
#include <iostream>

#include "lru/memoize.hpp"

__attribute__((annotate("memoize"))) std::size_t fib(std::size_t n) {
  if (n < 2) {
    return 1;
  }

  return fib(n - 1) + fib(n - 2);
}

int main() { std::cout << fib(50) << std::endl; }
