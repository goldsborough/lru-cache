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
