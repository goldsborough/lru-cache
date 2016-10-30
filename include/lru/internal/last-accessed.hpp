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

  LastAccessed() : _is_valid(false) {}

  void operator=(const Iterator &iterator) {
    _iterator = iterator;
    _is_valid = true;
  }

  friend bool
  operator==(const LastAccessed<Iterator> &iterator, const Key &key) noexcept {
    return iterator._is_valid && key == iterator._iterator->first;
  }

  friend bool operator==(const LastAccessed<Iterator> &iterator,
                         const Iterator &other) noexcept {
    return iterator._is_valid && other->first == iterator._iterator->first;
  }

  bool operator!=(const Key &key) const noexcept { return !(*this == key); }

  const Value &operator*() const noexcept {
    assert(_is_valid);
    return _iterator->second;
  }

  const Value *operator->() const noexcept { return &(*this); }

  explicit operator bool() const noexcept { return is_valid(); }

  operator Iterator() noexcept { return iterator(); }

  Iterator iterator() const noexcept { return _iterator; }

  auto is_valid() const noexcept { return _is_valid; }

  void invalidate() noexcept { _is_valid = false; }

 private:
  Iterator _iterator;
  bool _is_valid;
};
}
}

#endif /* LRU_INTERNAL_LAST_ACCESSED_HPP*/
