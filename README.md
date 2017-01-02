# lru-cache

A feature complete LRU cache implementation in C++.

## Description

A *least recently used* (LRU) cache is a fixed size cache that behaves just like a regular lookup table, but remembers the order in which elements are inserted. Once its (user-defined) capacity is reached, it uses this information to replace the least recently used element with a newly inserted one. This is ideal for caching function return values, where fast lookup of complex computation is favorable, but a memory blowup resulting from caching all `(input, output)` pairs is to be avoided.

We provide two implementations of an LRU cache: one has only the basic functionality described above, and another can be additionally supplied with a *time to live*. This is useful, for example, when caching resources on a server, where cache entries should be invalidated automatically after a certain amount of time, because they are no longer "fresh".

Additionally, all our caches can be connected to *statistics* objects, that keep track of cache hits and misses for all keys and, upon request, individual keys (similar to `functools.lru_cache` in Python).

## Basic Usage

The two main classes we provide are `LRU::Cache` and `LRU::TimedCache`. A basic usage example of these may look like so:

`LRU::Cache`:
```C++
#include <iostream>
#include "lru/lru.hpp"

using Cache = LRU::Cache<int, int>;

int fibonacci(int n, Cache& cache) {
  if (n < 2) return 1;

  // We internally keep track of the last accessed key, meaning a
  // `contains(key)` + `lookup(key)` sequence will involve only a single hash
  // table lookup.
  if (cache.contains(n)) return cache[n];

  auto value = fibonacci(n - 1, cache) + fibonacci(n - 2, cache);

  // Caches are 100% move-aware and we have implemented
  // `unordered_map` style emplacement and insertion.
  cache.emplace(n, value);

  return value;
}

int fibonacci(int n) {
  // Use a capacity of 100 (after 100 insertions, the next insertion will evict
  // the least-recently inserted element). The default capacity is 128.
  Cache cache(100);
  return fibonacci(n, cache);
}
```

`LRU::TimedCache`:
```C++
#include <chrono>
#include <iostream>

#include "lru/lru.hpp"

using namespace std::chrono_literals;

using Cache = LRU::TimedCache<int, int>;

int fibonacci(int n, Cache& cache) {
  if (n < 2) return 1;
  if (cache.contains(n)) return cache[n];

  auto value = fibonacci(n - 1, cache) + fibonacci(n - 2, cache);
  cache.emplace(n, value);

  return value;
}

int fibonacci(int n) {
  // Use a time to live of 100ms. This means that 100ms after insertion, a key
  // will be said to have "expired" and `contains(key)` will return false.
  Cache cache(100ms);
  return fibonacci(n, cache);
}

auto main() -> int {
  std::cout << fibonacci(32) << std::endl;
}
```

## Extended Usage
