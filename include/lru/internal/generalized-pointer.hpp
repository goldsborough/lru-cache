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

#include <algorithm>
#include <utility>

#ifndef LRU_INTERNAL_GENERALIZED_POINTER_HPP
#define LRU_INTERNAL_GENERALIZED_POINTER_HPP

namespace LRU {
namespace Internal {

template <typename T>
class GeneralizedPointer {
 public:
  GeneralizedPointer() noexcept : _is_owning(false), _pointer(nullptr) {
  }

  GeneralizedPointer(T&& value)  // NOLINT(runtime/explicit)
      : _is_owning(true),
        _pointer(new T(std::move(value))) {
  }

  GeneralizedPointer(T& value) noexcept  // NOLINT(runtime/explicit)
      : _is_owning(false),
        _pointer(&value) {
  }

  GeneralizedPointer(const GeneralizedPointer& other)
  : _is_owning(other._is_owning) {
    if (_is_owning) {
      _pointer = new T(*other._pointer);
    } else {
      _pointer = other._pointer;
    }
  }

  template <typename U,
            typename = std::enable_if_t<std::is_convertible<T, U>::value>>
  GeneralizedPointer(const GeneralizedPointer<U>& other)
  : _is_owning(other._is_owning) {
    if (_is_owning) {
      _pointer = new T(*other._pointer);
    } else {
      _pointer = other._pointer;
    }
  }

  GeneralizedPointer(GeneralizedPointer&& other) noexcept
  : _is_owning(other._is_owning), _pointer(other._pointer) {
  }

  template <typename U,
            typename = std::enable_if_t<std::is_convertible<T, U>::value>>
  GeneralizedPointer(GeneralizedPointer<U>&& other) noexcept
  : _is_owning(other._is_owning), _pointer(other._pointer) {
  }

  GeneralizedPointer& operator=(GeneralizedPointer other) {
    swap(other);
    return *this;
  }

  void swap(GeneralizedPointer& other) noexcept {
    using std::swap;

    swap(_pointer, other._pointer);
    swap(_is_owning, other._is_owning);
  }

  friend void
  swap(GeneralizedPointer& first, GeneralizedPointer& second) noexcept {
    first.swap(second);
  }

  ~GeneralizedPointer() {
    if (_is_owning) {
      std::cout << "delete" << std::endl;
      delete _pointer;
    }
  }

  T& operator*() noexcept {
    return value();
  }

  const T& operator*() const noexcept {
    return value();
  }

  T* operator->() noexcept {
    return _pointer;
  }

  const T* operator->() const noexcept {
    return _pointer;
  }

  T& value() noexcept {
    return *_pointer;
  }

  const T& value() const noexcept {
    return *_pointer;
  }

  T* get() noexcept {
    return _pointer;
  }

  const T* get() const noexcept {
    return _pointer;
  }

  explicit operator bool() const noexcept {
    return !is_null();
  }

  bool is_null() const noexcept {
    return _pointer == nullptr;
  }

  bool is_owning() const noexcept {
    return _is_owning;
  }

 private:
  T* _pointer;
  bool _is_owning;
};

}  // namespace Internal
}  // namespace LRU

#endif  // LRU_INTERNAL_GENERALIZED_POINTER_HPP
