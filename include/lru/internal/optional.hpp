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

#ifndef LRU_INTERNAL_OPTIONAL_HPP
#define LRU_INTERNAL_OPTIONAL_HPP

#ifndef __has_include
#define USE_LRU_OPTIONAL
#elif __has_include(<optional>)

#include <optional>
template <typename T>
using Optional = std::optional<T>;
#else
#define USE_LRU_OPTIONAL
#endif

#ifdef USE_LRU_OPTIONAL
#include <memory>
#include <stdexcept>

template <typename T>
class Optional {
 public:
  Optional() = default;


  Optional(const Optional& other) {
    reset(*other);
  }

  template <typename U>
  Optional(const Optional<U>& other) {
    reset(*other);
  }

  template <typename U>
  Optional(Optional<U>&& other) {
    reset(*std::forward<U>(other));
  }

  Optional(Optional&& other) {
    swap(other);
  }

  Optional& operator=(Optional other) noexcept {
    swap(other);
    return *this;
  }

  void swap(Optional& other) {
    _value.swap(other._value);
  }

  friend void swap(Optional& first, Optional& second) {
    first.swap(second);
  }

  constexpr explicit operator bool() const noexcept {
    return has_value();
  }

  constexpr bool has_value() const noexcept {
    return static_cast<bool>(_value);
  }

  constexpr const T* operator->() const {
    return _value.get();
  }

  constexpr T* operator->() {
    return _value.get();
  }

  constexpr const T& operator*() const {
    return *_value;
  }

  constexpr T& operator*() {
    return *_value;
  }

  constexpr T& value() {
    if (!has_value()) {
      // Actually std::bad_optional_access
      throw std::runtime_error("optional has no value");
    }

    return *_value;
  }

  constexpr const T& value() const {
    if (!has_value()) {
      // Actually std::bad_optional_access
      throw std::runtime_error("optional has no value");
    }

    return *_value;
  }

  template <class U>
  constexpr T value_or(U&& default_value) const {
    return *this ? **this : static_cast<T>(std::forward<U>(default_value));
  }

  void reset() {
    _value.reset();
  }

  template <typename... Args>
  void emplace(Args&&... args) {
    _value = std::make_unique<T>(std::forward<Args>(args)...);
  }

 private:
  std::unique_ptr<T> _value;
};

#endif

#endif // LRU_INTERNAL_OPTIONAL_HPP
