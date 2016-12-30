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

#ifndef LRU_INTERNAL_LAST_ACCESSED_HPP
#define LRU_INTERNAL_LAST_ACCESSED_HPP

#include <functional>
#include <iterator>

#include "lru/internal/utility.hpp"

namespace LRU {
namespace Internal {

/// Provides a simple iterator-compatible pointer object for a key and value.
///
/// The easisest idea for this class, theoretically, would be to just store an s
/// iterator to the internal cache map (i.e. template the class on the iterator
/// type). However, the major trouble with that approach is that this class
/// should be 100% *mutable*, as in "always non-const", so that  keys and values
/// we store for fast access can be (quickly) retrieved as either const or
/// non-const (iterators for example). This is not possible, since the
/// const-ness of `const_iterators` are not the usual idea of const in C++,
/// meaning especially it cannot be cast away with a `const_cast` as is required
/// for the mutability. As such, we *must* store the plain keys and values.
/// This, however, means that iterators cannot be stored efficiently, since a
/// new hash table lookup would be required to go from a key to its iterator.
/// However, since the main use case of this class is to avoid a second lookup
/// in the usual `if (cache.contains(key)) return cache.lookup(key)`, which is
/// not an issue for iterators since they can be compared to the `end` iterator
/// in constant time (equivalent to the call to `contains()`).
///
/// WARNING: This class stores *pointers* to keys and values. As such lifetime
/// of the pointed-to objects must be cared for by the user of this class.
template <typename Key, typename Value, typename KeyEqual = std::equal_to<Key>>
class LastAccessed {
 public:
  explicit LastAccessed(const KeyEqual& key_equal = KeyEqual())
  : _is_valid(false), _key_equal(key_equal) {
  }

  LastAccessed(const Key& key,
               const Value& value,
               const KeyEqual& key_equal = KeyEqual())
  : _key(const_cast<Key*>(&key))
  , _value(const_cast<Value*>(&value))
  , _is_valid(true)
  , _key_equal(key_equal) {
  }

  template <typename Iterator>
  explicit LastAccessed(Iterator iterator,
                        const KeyEqual& key_equal = KeyEqual())
  : LastAccessed(iterator->first, iterator->second, key_equal) {
  }

  template <typename Iterator>
  LastAccessed& operator=(Iterator iterator) {
    _key = const_cast<Key*>(&(iterator->first));
    _value = const_cast<Value*>(&(iterator->second));
    _is_valid = true;

    return *this;
  }

  /// Comparisons to keys
  friend bool
  operator==(const LastAccessed& last_accessed, const Key& key) noexcept {
    if (!last_accessed._is_valid) return false;
    return last_accessed._key_equal(key, *(last_accessed._key));
  }

  friend bool
  operator==(const Key& key, const LastAccessed& last_accessed) noexcept {
    return last_accessed == key;
  }

  friend bool
  operator!=(const LastAccessed& last_accessed, const Key& key) noexcept {
    return !(last_accessed == key);
  }

  friend bool
  operator!=(const Key& key, const LastAccessed& last_accessed) noexcept {
    return !(key == last_accessed);
  }

  /// Fast comparisons to other iterators (not relying on implicit conversions)
  template <typename Iterator, typename = enable_if_iterator<Iterator>>
  friend bool operator==(const LastAccessed& last_accessed,
                         const Iterator& iterator) noexcept {
    return last_accessed == iterator->first;
  }

  template <typename Iterator, typename = enable_if_iterator<Iterator>>
  friend bool operator==(const Iterator& iterator,
                         const LastAccessed& last_accessed) noexcept {
    return last_accessed == iterator;
  }

  template <typename Iterator, typename = enable_if_iterator<Iterator>>
  friend bool operator!=(const LastAccessed& last_accessed,
                         const Iterator& iterator) noexcept {
    return !(last_accessed == iterator);
  }

  template <typename Iterator, typename = enable_if_iterator<Iterator>>
  friend bool operator!=(const Iterator& iterator,
                         const LastAccessed& last_accessed) noexcept {
    return !(iterator == last_accessed);
  }

  explicit operator bool() const noexcept {
    return is_valid();
  }

  Key& key() noexcept {
    assert(is_valid());
    return *_key;
  }

  const Key& key() const noexcept {
    assert(is_valid());
    return *_key;
  }

  Value& value() noexcept {
    assert(is_valid());
    return *_value;
  }

  const Value& value() const noexcept {
    assert(is_valid());
    return *_value;
  }

  bool is_valid() const noexcept {
    return _is_valid;
  }

  void invalidate() noexcept {
    _is_valid = false;
  }

  void key_equal(const KeyEqual& key_equal) {
    _key_equal = key_equal;
  }

  const KeyEqual& key_equal() const noexcept {
    return _key_equal;
  }

 private:
  Key* _key;
  Value* _value;

  bool _is_valid;
  KeyEqual _key_equal;
};
}  // namespace Internal
}  // namespace LRU

#endif  // LRU_INTERNAL_LAST_ACCESSED_HPP
