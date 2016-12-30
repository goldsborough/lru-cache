# lru-cache

## TODO



## rewrite-tool
Some interesting links:
* http://stackoverflow.com/a/33473726
* http://clang.llvm.org/docs/InternalsManual.html#how-to-add-an-attribute
* http://llvm.org/releases/3.8.1/tools/docs/LibASTMatchersReference.html
* http://clang.llvm.org/doxygen/classclang_1_1Rewriter.html
* https://github.com/Microsoft/clang-tools-extra/blob/master/clang-tidy/modernize/PassByValueCheck.cpp
* http://stackoverflow.com/a/11154162
* http://stackoverflow.com/a/12662748

## Thoughts on Further Generalization

How about arbitrary decorators in Python?

Idea: We write a "meta tool" that can generate clang plugins for decorators. The steps would be:

1. The user defines decorators in C++. Such a decorator would not, like in Python, take a function and return the new function. Rather, the user would only have to define the body function that *will be called instead* of the original function. This function the user defines would have to:
  1. Take the original function as the first argument.
  2. Take any other arguments the function could be called with, or variadic arguments.
  3. Have a non-automatically-deduced templated return type.
For example, the following function would match these requirements:

```cpp
template <typename ReturnType, typename Function, typename... Ts>
ReturnType memoize(Function original_function, Ts &&... ts) {
  static std::unordered_map<std::tuple<Ts...>, ReturnType> map;
  auto key = std::make_tuple(ts...);
  auto iterator = map.find(key);

  if (iterator != map.end()) {
    return iterator->second;
  }

  auto value = original_function(std::forward<Ts>(ts)...);
  map.emplace(key, value);

  return value;
}
```
2. The user would then include this decorator where she wants to use them and annotate functions appropriately:
```cpp
#include "my-decorator.hpp"

int fib(int n) __attribute((anotate("memoize"))){
  if (n < 2) return 1;
  return fib(n - 1) + fib(n -2);
}
```
3. We would then write a tool, e.g. a Python script that renders a Jinja template, that generates a clang plugin, which does the following:
  1. Rename the matched function to `fib_original` or maybe a mangled name, to avoid collisions.
  2. Declare the original function with the original name before the now renamed function, so that recursive calls see the declaration.
  3. Place a new function under the original function, which has the original name and in whose body we call the template decorator defined by the user,  instantiated with the return type of the function.
In total, this will produce:
```cpp
// Original function declaration
int fib(int);

int __original__fib(int n) {
  if (n < 2) return 1;
  return fib(n - 1) + fib(n -2);
}

int fib(int n) {
  // Instantiate the template with the return type, which is now known
  return memoize<int>(__original__ fib, n);
}
```
4. Profit $$

## Extending Clang

The next step would be to actually add attributes to clang itself and recompile clang to use the attribute as if it were a standard attribute:

http://clang.llvm.org/docs/InternalsManual.html#attribute-basics
http://stackoverflow.com/questions/38391314/clang-custom-attributes-not-visible-in-ast
