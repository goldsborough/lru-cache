/**
* The MIT License (MIT)
* Copyright (c) 2016 Peter Goldsborough and Markus Engel
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#ifndef LRU_INTERNAL_LAST_ACCESSED_HPP
#define LRU_INTERNAL_LAST_ACCESSED_HPP

#include <iterator>

namespace LRU {
namespace Internal {

template <typename Iterator>
class LastAccessed {
 public:
  using Pair = typename std::iterator_traits<Iterator>::value_type;
  using Key = typename Pair::first_type;
  using Value = typename Pair::second_type;

  LastAccessed() : _is_valid(false) {
  }

  friend bool
  operator==(const LastAccessed &iterator, const Key &key) noexcept {
    return iterator._is_valid && key == iterator._iterator->first;
  }

  friend bool
  operator==(const Key &key, const LastAccessed &iterator) noexcept {
    return iterator == key;
  }

  friend bool
  operator==(const LastAccessed &iterator, const Iterator &other) noexcept {
    return iterator._is_valid && other->first == iterator._iterator->first;
  }

  friend bool
  operator==(const Iterator &other, const LastAccessed &iterator) noexcept {
    return iterator == other;
  }

  friend bool
  operator!=(const LastAccessed &iterator, const Key &key) noexcept {
    return !(iterator == key);
  }

  friend bool
  operator!=(const Key &key, const LastAccessed &iterator) noexcept {
    return !(iterator == key);
  }

  friend bool
  operator!=(const LastAccessed &iterator, const Iterator &other) noexcept {
    return !(iterator == other);
  }

  friend bool
  operator!=(const Iterator &other, const LastAccessed &iterator) noexcept {
    return !(iterator == other);
  }

  void operator=(const Iterator &iterator) {
    _iterator = iterator;
    _is_valid = true;
  }

  const Value &operator*() const noexcept {
    return value();
  }

  const Value *operator->() const noexcept {
    return &(value());
  }

  explicit operator bool() const noexcept {
    return is_valid();
  }

  operator Iterator() noexcept {
    return iterator();
  }

  Iterator iterator() const noexcept {
    return _iterator;
  }

  const Value &value() const noexcept {
    assert(_is_valid);
    return _iterator->second;
  }

  auto is_valid() const noexcept {
    return _is_valid;
  }

  void invalidate() noexcept {
    _is_valid = false;
  }

 private:
  Iterator _iterator;

  bool _is_valid;
};
}
}

#endif /* LRU_INTERNAL_LAST_ACCESSED_HPP*/
