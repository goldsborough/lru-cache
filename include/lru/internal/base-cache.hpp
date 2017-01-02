/// The MIT License (MIT)
/// Copyright (c) 2016 Peter Goldsborough
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

#ifndef LRU_INTERNAL_BASE_CACHE_HPP
#define LRU_INTERNAL_BASE_CACHE_HPP

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <list>
#include <memory>
#include <tuple>
#include <unordered_map>
#include <utility>

#include "lru/insertion-result.hpp"
#include "lru/internal/base-ordered-iterator.hpp"
#include "lru/internal/base-unordered-iterator.hpp"
#include "lru/internal/definitions.hpp"
#include "lru/internal/last-accessed.hpp"
#include "lru/internal/optional.hpp"
#include "lru/internal/statistics-mutator.hpp"
#include "lru/internal/utility.hpp"
#include "lru/statistics.hpp"

namespace LRU {
namespace Internal {

// Macros are bad, but also more readable sometimes:
// Without this macro, it becomes a pain to have a `using` directive for every
// new member we add to the `BaseCache` and rename or remove every such
// directive when we make a change to the `BaseCache`.
// With this macro, you can simply do:
// using super = BaseCache<Key, Value, Information>;
// using BASE_CACHE_MEMBERS;
#define PUBLIC_BASE_CACHE_MEMBERS               \
  super::is_full;                               \
  using super::is_empty;                        \
  using super::clear;                           \
  using super::end;                             \
  using super::cend;                            \
  using super::operator=;                       \
  using typename super::Information;            \
  using typename super::UnorderedIterator;      \
  using typename super::UnorderedConstIterator; \
  using typename super::OrderedIterator;        \
  using typename super::OrderedConstIterator;   \
  using typename super::InitializerList;

#define PRIVATE_BASE_CACHE_MEMBERS           \
  super::_map;                               \
  using typename super::Map;                 \
  using typename super::MapIterator;         \
  using typename super::MapConstIterator;    \
  using typename super::Queue;               \
  using typename super::QueueIterator;       \
  using super::_order;                       \
  using super::_last_accessed;               \
  using super::_capacity;                    \
  using super::_erase;                       \
  using super::_erase_lru;                   \
  using super::_move_to_front;               \
  using super::_value_from_result;           \
  using super::_last_accessed_is_ok;         \
  using super::_register_miss_if_monitoring; \
  using super::_register_hit_if_monitoring;

/// The base class for the LRU::Cache and LRU::TimedCache.
///
/// This base class (base as opposed to abstract, because it is not intended to
/// be used polymorphically) provides the great bulk of the implementation of
/// both the LRU::Cache and the timed version. For example, it builds the
/// `contains()`, `lookup()` and `operator[]()` functions on top of the pure
/// virtual `find()` methods, making the final implementation of the LRU::Cache
/// much less strenuous.
///
/// This class also defines all concrete iterator classes and provides the main
/// iterator interface of all caches via ordered and unordered iterators and
/// appropriate `begin()`, `end()` and similar methods.
///
/// Lastly, the `BaseCache` provides a statistics interface to register and
/// access shared or owned statistics.
///
/// \tparam Key The key type of the cache.
/// \tparam Value The value type of the cache.
/// \tparam InformationType The internal information class to be used.
/// \tparam HashFunction The hash function type for the internal map.
/// \tparam KeyEqual The type of the key equality function for the internal map.
/// \tparam TagType The cache tag type of the concrete derived class.
template <typename Key,
          typename Value,
          template <typename, typename> class InformationType,
          typename HashFunction,
          typename KeyEqual,
          typename TagType>
class BaseCache {
 protected:
  using Information = InformationType<Key, Value>;
  using Queue = Internal::Queue<Key>;
  using QueueIterator = typename Queue::const_iterator;

  using Map = Internal::Map<Key, Information, HashFunction, KeyEqual>;
  using MapIterator = typename Map::iterator;
  using MapConstIterator = typename Map::const_iterator;

 public:
  using Tag = TagType;
  using InitializerList = std::initializer_list<std::pair<Key, Value>>;
  using StatisticsPointer = std::shared_ptr<Statistics<Key>>;
  using size_t = std::size_t;

  static constexpr Tag tag() noexcept {
    return {};
  }

  /////////////////////////////////////////////////////////////////////////////
  // ITERATORS CLASSES
  /////////////////////////////////////////////////////////////////////////////

  /// A non-const unordered iterator.
  ///
  /// Unordered iterators provide faster lookup than ordered iterators because
  /// they have direct access to the underlying map. Also, they can convert to
  /// ordered iterators cheaply.
  struct UnorderedIterator
      : public BaseUnorderedIterator<BaseCache, MapIterator> {
    using super = BaseUnorderedIterator<BaseCache, MapIterator>;
    friend BaseCache;

    /// Default constructor.
    UnorderedIterator() = default;

    /// Constructs a new UnorderedIterator from an unordered base iterator.
    ///
    /// \param iterator The iterator to initialize this one from.
    UnorderedIterator(BaseUnorderedIterator<BaseCache, MapIterator>
                          iterator)  // NOLINT(runtime/explicit)
        : super(std::move(iterator)) {
      // Note that this only works because these derived iterator
      // classes dont' have any members of their own.
      // It is necessary because the increment operators return base iterators.
    }

    /// Constructs a new UnorderedIterator.
    ///
    /// \param cache The cache this iterator references.
    /// \param iterator The underlying map iterator.
    UnorderedIterator(BaseCache& cache,
                      MapIterator iterator)  // NOLINT(runtime/explicit)
        : super(cache, iterator) {
    }
  };

  /// A const unordered iterator.
  ///
  /// Unordered iterators provide faster lookup than ordered iterators because
  /// they have direct access to the underlying map. Also, they can convert to
  /// ordered iterators cheaply.
  struct UnorderedConstIterator
      : public BaseUnorderedIterator<const BaseCache, MapConstIterator> {
    using super = BaseUnorderedIterator<const BaseCache, MapConstIterator>;
    friend BaseCache;

    /// Default constructor.
    UnorderedConstIterator() = default;

    /// Constructs a new UnorderedConstIterator from any unordered base
    /// iterator.
    ///
    /// \param iterator The iterator to initialize this one from.
    template <typename AnyCache, typename AnyUnderlyingIterator>
    UnorderedConstIterator(
        BaseUnorderedIterator<AnyCache, AnyUnderlyingIterator> iterator)
    : super(std::move(iterator)) {
      // Note that this only works because these derived iterator
      // classes dont' have any members of their own.
    }

    /// Constructs a new UnorderedConstIterator from a non-const iterator.
    ///
    /// \param iterator The non-const iterator to initialize this one from.
    UnorderedConstIterator(
        UnorderedIterator iterator)  // NOLINT(runtime/explicit)
        : super(std::move(iterator)) {
    }

    /// Constructs a new UnorderedConstIterator.
    ///
    /// \param cache The cache this iterator references.
    /// \param iterator The underlying map iterator.
    UnorderedConstIterator(
        const BaseCache& cache,
        MapConstIterator iterator)  // NOLINT(runtime/explicit)
        : super(cache, iterator) {
    }
  };

  /// An ordered iterator.
  ///
  /// Ordered iterators have a performance disadvantage compared to unordered
  /// iterators the first time they are dereferenced. However, they may be
  /// constructed or assigned from unordered iterators (of compatible
  /// qualifiers).
  struct OrderedIterator : public BaseOrderedIterator<Key, Value, BaseCache> {
    using super = BaseOrderedIterator<Key, Value, BaseCache>;
    using UnderlyingIterator = typename super::UnderlyingIterator;
    friend BaseCache;

    /// Default constructor.
    OrderedIterator() = default;

    /// Constructs an ordered iterator from an unordered iterator.
    ///
    /// \param unordered_iterator The unordered iterator to construct from.
    explicit OrderedIterator(UnorderedIterator unordered_iterator)
    : super(std::move(unordered_iterator)) {
    }

    /// Constructs a new OrderedIterator from an unordered base iterator.
    ///
    /// \param iterator The iterator to initialize this one from.
    OrderedIterator(BaseOrderedIterator<Key, Value, BaseCache>
                        iterator)  // NOLINT(runtime/explicit)
        : super(std::move(iterator)) {
      // Note that this only works because these derived iterator
      // classes dont' have any members of their own.
      // It is necessary because the increment operators return base iterators.
    }

    /// Constructs a new ordered iterator.
    ///
    /// \param cache The cache this iterator references.
    /// \param iterator The underlying iterator.
    OrderedIterator(BaseCache& cache, UnderlyingIterator iterator)
    : super(cache, iterator) {
    }
  };

  /// A const ordered iterator.
  ///
  /// Ordered iterators have a performance disadvantage compared to unordered
  /// iterators the first time they are dereferenced. However, they may be
  /// constructed or assigned from unordered iterators (of compatible
  /// qualifiers).
  struct OrderedConstIterator
      : public BaseOrderedIterator<Key, const Value, const BaseCache> {
    using super = BaseOrderedIterator<Key, const Value, const BaseCache>;
    using UnderlyingIterator = typename super::UnderlyingIterator;

    friend BaseCache;

    /// Default constructor.
    OrderedConstIterator() = default;

    /// Constructs a new OrderedConstIterator from a compatible ordered
    /// iterator.
    ///
    /// \param iterator The iterator to initialize this one from.
    template <typename AnyKey, typename AnyValue, typename AnyCache>
    OrderedConstIterator(BaseOrderedIterator<AnyKey, AnyValue, AnyCache>
                             iterator)  // NOLINT(runtime/explicit)
        : super(iterator) {
      // Note that this only works because these derived iterator
      // classes dont' have any members of their own.
    }

    /// Constructs a new const ordered iterator from a non-const one.
    ///
    /// \param iterator The non-const ordered iterator to construct from.
    OrderedConstIterator(OrderedIterator iterator)  // NOLINT(runtime/explicit)
        : super(std::move(iterator)) {
    }

    /// Constructs a new const ordered iterator from an unordered iterator.
    ///
    /// \param unordered_iterator The unordered iterator to construct from.
    explicit OrderedConstIterator(UnorderedIterator unordered_iterator)
    : super(std::move(unordered_iterator)) {
    }

    /// Constructs a new const ordered iterator from a const unordered iterator.
    ///
    /// \param unordered_iterator The unordered iterator to construct from.
    explicit OrderedConstIterator(
        UnorderedConstIterator unordered_iterator)  // NOLINT(runtime/explicit)
        : super(std::move(unordered_iterator)) {
    }

    /// Constructs a new const ordered iterator.
    ///
    /// \param cache The cache this iterator references.
    /// \param iterator The underlying iterator.
    OrderedConstIterator(const BaseCache& cache, UnderlyingIterator iterator)
    : super(cache, iterator) {
    }
  };

  using InsertionResultType = InsertionResult<UnorderedIterator>;

  /////////////////////////////////////////////////////////////////////////////
  // SPECIAL MEMBER FUNCTIONS
  /////////////////////////////////////////////////////////////////////////////

  /// Constructor.
  ///
  /// \param capacity The capacity of the cache.
  /// \param hash The hash function to use for the internal map.
  /// \param key_equal The key equality function to use for the internal map.
  BaseCache(size_t capacity,
            const HashFunction& hash,
            const KeyEqual& key_equal)
  : _map(0, hash, key_equal), _capacity(capacity), _last_accessed(key_equal) {
  }

  /// Constructor.
  ///
  /// \param capacity The capacity of the cache.
  /// \param begin The start of a range to construct the cache with.
  /// \param end The end of a range to construct the cache with.
  /// \param hash The hash function to use for the internal map.
  /// \param key_equal The key equality function to use for the internal map.
  template <typename Iterator>
  BaseCache(size_t capacity,
            Iterator begin,
            Iterator end,
            const HashFunction& hash,
            const KeyEqual& key_equal)
  : BaseCache(capacity, hash, key_equal) {
    insert(begin, end);
  }

  /// Constructor.
  ///
  /// The capacity is inferred from the distance between the two iterators and
  /// lower-bounded by an internal constant $c_0$, usually 128 (i.e. the actual
  /// capacity will be $\max(\text{distance}, c_0)$).
  /// This may be expensive for iterators that are not random-access.
  ///
  /// \param begin The start of a range to construct the cache with.
  /// \param end The end of a range to construct the cache with.
  /// \param hash The hash function to use for the internal map.
  /// \param key_equal The key equality function to use for the internal map.
  template <typename Iterator>
  BaseCache(Iterator begin,
            Iterator end,
            const HashFunction& hash,
            const KeyEqual& key_equal)
      // This may be expensive
      : BaseCache(std::max<size_t>(std::distance(begin, end),
                                   Internal::DEFAULT_CAPACITY),
                  begin,
                  end,
                  hash,
                  key_equal) {
  }

  /// Constructor.
  ///
  /// \param capacity The capacity of the cache.
  /// \param range A range to construct the cache with.
  /// \param hash The hash function to use for the internal map.
  /// \param key_equal The key equality function to use for the internal map.
  template <typename Range, typename = Internal::enable_if_range<Range>>
  BaseCache(size_t capacity,
            Range& range,
            const HashFunction& hash,
            const KeyEqual& key_equal)
  : BaseCache(capacity, hash, key_equal) {
    insert(range);
  }

  /// Constructor.
  ///
  /// The capacity is inferred from the distance between the beginning and end
  /// of the range. This may be expensive for iterators that are not
  /// random-access.
  ///
  /// \param range A range to construct the cache with.
  /// \param hash The hash function to use for the internal map.
  /// \param key_equal The key equality function to use for the internal map.
  template <typename Range, typename = Internal::enable_if_range<Range>>
  explicit BaseCache(Range& range,
                     const HashFunction& hash,
                     const KeyEqual& key_equal)
  : BaseCache(std::begin(range), std::end(range), hash, key_equal) {
  }

  /// Constructor.
  ///
  /// Elements of the range will be moved into the cache.
  ///
  /// \param capacity The capacity of the cache.
  /// \param range A range to construct the cache with.
  /// \param hash The hash function to use for the internal map.
  /// \param key_equal The key equality function to use for the internal map.
  template <typename Range, typename = Internal::enable_if_range<Range>>
  BaseCache(size_t capacity,
            Range&& range,
            const HashFunction& hash,
            const KeyEqual& key_equal)
  : BaseCache(capacity, hash, key_equal) {
    insert(std::move(range));
  }

  /// Constructor.
  ///
  /// The capacity is inferred from the distance between the beginning and end
  /// of the range. This may be expensive for iterators that are not
  /// random-access.
  ///
  /// Elements of the range will be moved into the cache.
  ///
  /// \param range A range to construct the cache with.
  /// \param hash The hash function to use for the internal map.
  /// \param key_equal The key equality function to use for the internal map.
  template <typename Range, typename = Internal::enable_if_range<Range>>
  explicit BaseCache(Range&& range,
                     const HashFunction& hash,
                     const KeyEqual& key_equal)
  : BaseCache(std::distance(std::begin(range), std::end(range)),
              std::move(range),
              hash,
              key_equal) {
  }

  /// Constructor.
  ///
  /// \param capacity The capacity of the cache.
  /// \param list The initializer list to construct the cache with.
  /// \param hash The hash function to use for the internal map.
  /// \param key_equal The key equality function to use for the internal map.
  BaseCache(size_t capacity,
            InitializerList list,
            const HashFunction& hash,
            const KeyEqual& key_equal)
  : BaseCache(capacity, list.begin(), list.end(), hash, key_equal) {
  }

  /// Constructor.
  ///
  /// \param list The initializer list to construct the cache with.
  /// \param hash The hash function to use for the internal map.
  /// \param key_equal The key equality function to use for the internal map.
  BaseCache(InitializerList list,
            const HashFunction& hash,
            const KeyEqual& key_equal)  // NOLINT(runtime/explicit)
      : BaseCache(list.size(), list.begin(), list.end(), hash, key_equal) {
  }

  /// Copy constructor.
  BaseCache(const BaseCache& other) = default;

  /// Move constructor.
  BaseCache(BaseCache&& other) = default;

  /// Copy assignment operator.
  BaseCache& operator=(const BaseCache& other) = default;

  /// Move assignment operator.
  BaseCache& operator=(BaseCache&& other) = default;

  /// Destructor.
  virtual ~BaseCache() = default;

  /// Sets the contents of the cache to a range.
  ///
  /// If the size of the range is greater than the current capacity,
  /// the capacity is increased to match the range's size. If the size of
  /// the range is less than the current capacity, the cache's capacity is *not*
  /// changed.
  ///
  /// \param range A range of pairs to assign to the cache.
  /// \returns The cache instance.
  template <typename Range, typename = Internal::enable_if_range<Range>>
  BaseCache& operator=(const Range& range) {
    _clear_and_increase_capacity(range);
    insert(range);
    return *this;
  }

  /// Sets the contents of the cache to an rvalue range.
  ///
  /// Pairs of the range are moved into the cache.
  ///
  /// \param range A range of pairs to assign to the cache.
  /// \returns The cache instance.
  template <typename Range, typename = Internal::enable_if_range<Range>>
  BaseCache& operator=(Range&& range) {
    _clear_and_increase_capacity(range);
    insert(std::move(range));
    return *this;
  }

  /// Sets the contents of the cache to pairs from a list.
  ///
  /// \param list The list to assign to the cache.
  /// \returns The cache instance.
  BaseCache& operator=(InitializerList list) {
    return operator=<InitializerList>(list);
  }

  /// Swaps the contents of the cache with another cache.
  ///
  /// \param other The other cache to swap with.
  virtual void swap(BaseCache& other) noexcept {
    using std::swap;

    swap(_order, other._order);
    swap(_map, other._map);
    swap(_last_accessed, other._last_accessed);
    swap(_capacity, other._capacity);
  }

  /// Swaps the contents of one cache with another cache.
  ///
  /// \param first The first cache to swap.
  /// \param second The second cache to swap.
  friend void swap(BaseCache& first, BaseCache& second) noexcept {
    first.swap(second);
  }

  /// Compares the cache for equality with another cache.
  ///
  /// \complexity O(N)
  /// \param other The other cache to compare with.
  /// \returns True if the keys __and values__ of the cache are identical to the
  ///         other, else false.
  bool operator==(const BaseCache& other) const noexcept {
    if (this == &other) return true;
    if (this->_map != other._map) return false;
    if (this->_order != other._order) return false;
    return true;
  }

  /// Compares the cache for inequality with another cache.
  ///
  /// \complexity O(N)
  /// \param other The other cache to compare with.
  /// \returns True if there is any mismatch in keys __or their values__
  /// betweent
  /// the two caches, else false.
  bool operator!=(const BaseCache& other) const noexcept {
    return !(*this == other);
  }

  /////////////////////////////////////////////////////////////////////////////
  // ITERATOR INTERFACE
  /////////////////////////////////////////////////////////////////////////////

  /// \returns An unordered iterator to the beginning of the cache (this need
  /// not be the first key inserted).
  UnorderedIterator unordered_begin() noexcept {
    return {*this, _map.begin()};
  }

  /// \returns A const unordered iterator to the beginning of the cache (this
  /// need not be the key least recently inserted).
  UnorderedConstIterator unordered_begin() const noexcept {
    return unordered_cbegin();
  }

  /// \returns A const unordered iterator to the beginning of the cache (this
  /// need not be the key least recently inserted).
  UnorderedConstIterator unordered_cbegin() const noexcept {
    return {*this, _map.cbegin()};
  }

  /// \returns An unordered iterator to the end of the cache (this
  /// need not be one past the key most recently inserted).
  UnorderedIterator unordered_end() noexcept {
    return {*this, _map.end()};
  }

  /// \returns A const unordered iterator to the end of the cache (this
  /// need not be one past the key most recently inserted).
  UnorderedConstIterator unordered_end() const noexcept {
    return unordered_cend();
  }

  /// \returns A const unordered iterator to the end of the cache (this
  /// need not be one past the key most recently inserted).
  UnorderedConstIterator unordered_cend() const noexcept {
    return {*this, _map.cend()};
  }

  /// \returns An ordered iterator to the beginning of the cache (the key least
  /// recently inserted).
  OrderedIterator ordered_begin() noexcept {
    return {*this, _order.begin()};
  }

  /// \returns A const ordered iterator to the beginning of the cache (the key
  /// least recently inserted).
  OrderedConstIterator ordered_begin() const noexcept {
    return ordered_cbegin();
  }

  /// \returns A const ordered iterator to the beginning of the cache (the key
  /// least recently inserted).
  OrderedConstIterator ordered_cbegin() const noexcept {
    return {*this, _order.cbegin()};
  }

  /// \returns An ordered iterator to the end of the cache (one past the key
  /// most recently inserted).
  OrderedIterator ordered_end() noexcept {
    return {*this, _order.end()};
  }

  /// \returns A const ordered iterator to the end of the cache (one past the
  /// key least recently inserted).
  OrderedConstIterator ordered_end() const noexcept {
    return ordered_cend();
  }

  /// \returns A const ordered iterator to the end of the cache (one past the
  /// key least recently inserted).
  OrderedConstIterator ordered_cend() const noexcept {
    return {*this, _order.cend()};
  }

  /// \copydoc unordered_begin()
  UnorderedIterator begin() noexcept {
    return unordered_begin();
  }

  /// \copydoc unordered_cbegin()
  UnorderedConstIterator begin() const noexcept {
    return cbegin();
  }

  /// \copydoc unordered_cbegin()
  UnorderedConstIterator cbegin() const noexcept {
    return unordered_begin();
  }

  /// \copydoc unordered_end() const
  UnorderedIterator end() noexcept {
    return unordered_end();
  }

  /// \copydoc unordered_cend() const
  UnorderedConstIterator end() const noexcept {
    return cend();
  }

  /// \copydoc unordered_cend() const
  UnorderedConstIterator cend() const noexcept {
    return unordered_cend();
  }

  /// \returns True if the given iterator may be safely dereferenced, else
  /// false.
  /// \details Behavior is undefined if the iterator does not point into this
  /// cache.
  /// \param unordered_iterator The iterator to check.
  virtual bool is_valid(UnorderedConstIterator unordered_iterator) const
      noexcept {
    return unordered_iterator != unordered_end();
  }

  /// \returns True if the given iterator may be safely dereferenced, else
  /// false.
  /// \details Behavior is undefined if the iterator does not point into this
  /// cache.
  /// \param ordered_iterator The iterator to check.
  virtual bool is_valid(OrderedConstIterator ordered_iterator) const noexcept {
    return ordered_iterator != ordered_end();
  }

  /// Checks if the given iterator may be dereferencend and throws an exception
  /// if not.
  ///
  /// The exception thrown, if any, depends on the state of the iterator.
  ///
  /// \param unordered_iterator The iterator to check.
  /// \throws LRU::Error::InvalidIterator if the iterator is the end iterator.
  virtual void
  throw_if_invalid(UnorderedConstIterator unordered_iterator) const {
    if (unordered_iterator == unordered_end()) {
      throw LRU::Error::InvalidIterator();
    }
  }

  /// Checks if the given iterator may be dereferencend and throws an exception
  /// if not.
  ///
  /// The exception thrown, if any, depends on the state of the iterator.
  ///
  /// \param ordered_iterator The iterator to check.
  /// \throws LRU::Error::InvalidIterator if the iterator is the end iterator.
  virtual void throw_if_invalid(OrderedConstIterator ordered_iterator) const {
    if (ordered_iterator == ordered_end()) {
      throw LRU::Error::InvalidIterator();
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  // CACHE INTERFACE
  /////////////////////////////////////////////////////////////////////////////

  /// Tests if the given key is contained in the cache.
  ///
  /// This function may return false even if the key is actually currently
  /// stored in the cache, but the concrete cache class places some additional
  /// constraint as to when a key may be accessed (such as a time limit).
  ///
  /// \complexity O(1) expected and amortized.
  /// \param key The key to check for.
  /// \returns True if the key's value may be accessed via `lookup()` without an
  /// error, else false.
  virtual bool contains(const Key& key) const {
    if (key == _last_accessed) {
      if (_last_accessed_is_ok(key)) {
        _register_hit_if_monitoring(key);
        return true;
      } else {
        return false;
      }
    }

    auto iterator = find(key);
    if (iterator != end()) {
      _last_accessed = iterator._iterator;
      return true;
    } else {
      return false;
    }
  }

  /// Looks up the value for the given key.
  ///
  /// \complexity O(1) expected and amortized.
  /// \param key The key whose value to look for.
  /// \throws LRU::Error::KeyNotFound if the key's value may not be accessed.
  /// \returns The value stored in the cache for the given key.
  /// \see contains()
  virtual const Value& lookup(const Key& key) const {
    if (key == _last_accessed) {
      _register_hit_if_monitoring(key);
      return _value_for_last_accessed();
    }

    auto iterator = find(key);
    if (iterator == end()) {
      throw LRU::Error::KeyNotFound();
    } else {
      return iterator.value();
    }
  }

  /// Looks up the value for the given key.
  ///
  /// \complexity O(1) expected and amortized.
  /// \param key The key whose value to look for.
  /// \throws LRU::Error::KeyNotFound if the key's value may not be accessed.
  /// \returns The value stored in the cache for the given key.
  /// \see contains()
  virtual Value& lookup(const Key& key) {
    if (key == _last_accessed) {
      _register_hit_if_monitoring(key);
      return _value_for_last_accessed();
    }

    auto iterator = find(key);
    if (iterator == end()) {
      throw LRU::Error::KeyNotFound();
    } else {
      return iterator.value();
    }
  }

  /// Attempts to return an iterator to the given key in the cache.
  ///
  /// \complexity O(1) expected and amortized.
  /// \param key The key whose value to look for.
  /// \returns An iterator pointing to the entry with the given key, if one
  /// exists, else the end iterator.
  virtual UnorderedIterator find(const Key& key) = 0;

  /// Attempts to return a const iterator to the given key in the cache.
  ///
  /// \complexity O(1) expected and amortized.
  /// \param key The key whose value to look for.
  /// \returns A const iterator pointing to the entry with the given key, if one
  /// exists, else the end iterator.
  virtual UnorderedConstIterator find(const Key& key) const = 0;

  /// \copydoc lookup(const Key&)
  virtual Value& operator[](const Key& key) {
    return lookup(key);
  }

  /// \copydoc lookup(const Key&) const
  virtual const Value& operator[](const Key& key) const {
    return lookup(key);
  }

  /// Inserts the given `(key, value)` pair into the cache.
  ///
  /// \complexity O(1) expected and amortized.
  /// \param key The key to insert.
  /// \param value The value to insert with the key.
  /// \returns An `InsertionResult`, holding a boolean indicating whether the
  /// key was newly inserted (true) or only updated (false) as well as an
  /// iterator pointing to the entry for the key.
  virtual InsertionResultType insert(const Key& key, const Value& value) {
    if (_capacity == 0) return {false, end()};

    auto iterator = _map.find(key);

    // To insert, we first check if the key is already present in the cache
    // and if so, update its value and move its order iterator to the front
    // of the queue. Else, we insert the key at the end of the queue and
    // possibly pop the front if the cache has reached its capacity.

    if (iterator == _map.end()) {
      if (is_full()) {
        _erase_lru();
      }

      auto order = _order.insert(_order.end(), key);
      auto result = _map.emplace(key, Information(order, value));
      assert(result.second);

      _last_accessed = result.first;
      return {true, {*this, result.first}};
    } else {
      _move_to_front(iterator, value);
      return {false, {*this, iterator}};
    }
  }

  /// Inserts a range of `(key, value)` pairs.
  ///
  /// This operation has no performance benefits over
  /// element-wise insertion via `insert()`.
  ///
  /// \param begin An iterator for the start of the range to insert.
  /// \param end An iterator for the end of the range to insert.
  /// \returns The number of elements newly inserted (as opposed to only
  /// updated).
  template <typename Iterator,
            typename = Internal::enable_if_iterator_over_pair<Iterator>>
  size_t insert(Iterator begin, Iterator end) {
    size_t newly_inserted = 0;
    for (; begin != end; ++begin) {
      const auto result = insert(begin->first, begin->second);
      newly_inserted += result.was_inserted();
    }

    return newly_inserted;
  }

  /// Inserts a range of `(key, value)` pairs.
  ///
  /// This operation has no performance benefits over
  /// element-wise insertion via `insert()`.
  ///
  /// \param range The range of `(key, value)` pairs to insert.
  /// \returns The number of elements newly inserted (as opposed to only
  /// updated).
  template <typename Range, typename = Internal::enable_if_range<Range>>
  size_t insert(Range& range) {
    using std::begin;
    using std::end;

    return insert(begin(range), end(range));
  }

  /// Moves the elements of the range into the cache.
  ///
  /// \param range The range of `(key, value)` pairs to move into the cache.
  /// \returns The number of elements newly inserted (as opposed to only
  /// updated).
  template <typename Range, typename = Internal::enable_if_range<Range>>
  size_t insert(Range&& range) {
    size_t newly_inserted = 0;
    for (auto& pair : range) {
      const auto result =
          emplace(std::move(pair.first), std::move(pair.second));
      newly_inserted += result.was_inserted();
    }

    return newly_inserted;
  }

  /// Inserts a list `(key, value)` pairs.
  ///
  /// This operation has no performance benefits over
  /// element-wise insertion via `insert()`.
  ///
  /// \param list The list of `(key, value)` pairs to insert.
  /// \returns The number of elements newly inserted (as opposed to only
  /// updated).
  virtual size_t insert(InitializerList list) {
    return insert(list.begin(), list.end());
  }

  /// Emplaces a new `(key, value)` pair into the cache.
  ///
  /// This emplacement function allows perfectly forwarding an arbitrary number
  /// of arguments to the constructor of both the key and value type, via
  /// appropriate tuples. The intended usage is with `std::forward_as_tuple`,
  /// for example:
  /// \code{.cpp}
  /// struct A { A(int, const std::string&) { } };
  /// struct B { B(double) {} };
  ///
  /// LRU::Cache<A> cache;
  ///
  /// cache.emplace(
  ///   std::piecewise_construct,
  ///   std::forward_as_tuple(1, "hello"),
  ///   std::forward_as_tuple(5.0),
  ///  );
  /// \endcode
  ///
  /// There is a convenience overload that requires much less overhead, if both
  /// constructors expect only a single argument.
  ///
  /// \complexity O(1) expected and amortized.
  /// \param _ A dummy parameter to work around overload resolution.
  /// \param key_arguments A tuple of arguments to construct a key object with.
  /// \param value_arguments A tuple of arguments to construct a value object
  ///                        with.
  /// \returns An `InsertionResult`, holding a boolean indicating whether the
  /// key was newly inserted (true) or only updated (false) as well as an
  /// iterator pointing to the entry for the key.
  template <typename... Ks, typename... Vs>
  InsertionResultType emplace(std::piecewise_construct_t _,
                              const std::tuple<Ks...>& key_arguments,
                              const std::tuple<Vs...>& value_arguments) {
    if (_capacity == 0) return {false, end()};

    auto key = Internal::construct_from_tuple<Key>(key_arguments);
    auto iterator = _map.find(key);

    if (iterator == _map.end()) {
      if (is_full()) {
        _erase_lru();
      }

      auto order = _order.emplace(_order.end(), key);

      // clang-format off
        auto result = _map.emplace(
          std::move(key),
          Information(order, value_arguments)
        );
      // clang-format on

      assert(result.second);

      _last_accessed = result.first;

      return {true, {*this, result.first}};
    } else {
      auto value = Internal::construct_from_tuple<Value>(value_arguments);
      _move_to_front(iterator, value);
      return {false, {*this, iterator}};
    }
  }

  /// Emplaces a `(key, value)` pair.
  ///
  /// This is a convenience overload removing the necessity for
  /// `std::piecewise_construct` and `std::forward_as_tuple` that may be used in
  /// the case that both the key and value have constructors expecting only a
  /// single argument.
  ///
  /// \param key_argument The argument to construct a key object with.
  /// \param value_argument The argument to construct a value object with.
  /// \returns An `InsertionResult`, holding a boolean indicating whether the
  /// key was newly inserted (true) or only updated (false) as well as an
  /// iterator pointing to the entry for the key.
  template <typename K, typename V>
  InsertionResultType emplace(K&& key_argument, V&& value_argument) {
    auto key_tuple = std::forward_as_tuple(std::forward<K>(key_argument));
    auto value_tuple = std::forward_as_tuple(std::forward<V>(value_argument));
    return emplace(std::piecewise_construct, key_tuple, value_tuple);
  }

  /// Erases the given key from the cache, if it is present.
  ///
  /// If the key is not present in the cache, this is a no-op.
  ///
  /// \param key The key to erase.
  /// \returns True if the key was erased, else false.
  virtual bool erase(const Key& key) {
    // No need to use _last_accessed_is_ok here, because even
    // if it has expired, it's no problem to erase it anyway
    if (_last_accessed == key) {
      _erase(_last_accessed.key(), _last_accessed.information());
      return true;
    }

    auto iterator = _map.find(key);
    if (iterator != _map.end()) {
      _erase(iterator);
      return true;
    }

    return false;
  }

  /// Erases the key pointed to by the given iterator.
  ///
  /// \param iterator The iterator whose key to erase.
  /// \throws LRU::Error::InvalidIterator if the iterator is the end iterator.
  virtual void erase(UnorderedConstIterator iterator) {
    /// We have this overload to avoid the extra conversion-construction from
    /// unordered to ordered iterator (and renewed hash lookup)
    if (iterator == unordered_cend()) {
      throw LRU::Error::InvalidIterator();
    } else {
      _erase(iterator._iterator);
    }
  }


  /// Erases the key pointed to by the given iterator.
  ///
  /// \param iterator The iterator whose key to erase.
  /// \throws LRU::Error::InvalidIterator if the iterator is the end iterator.
  virtual void erase(OrderedConstIterator iterator) {
    if (iterator == ordered_cend()) {
      throw LRU::Error::InvalidIterator();
    } else {
      _erase(_map.find(iterator.key()));
    }
  }

  /// Clears the cache entirely.
  virtual void clear() {
    _map.clear();
    _order.clear();
    _last_accessed.invalidate();
  }

  /// Requests shrinkage of the cache to the given size.
  ///
  /// If the passed size is 0, this operation is equivalent to `clear()`. If the
  /// size is greater than the current size, it is a no-op. Otherwise, the size
  /// of the cache is reduzed to the given size by repeatedly removing the least
  /// recent element.
  ///
  /// \param new_size The size to (maybe) shrink to.
  virtual void shrink(size_t new_size) {
    if (new_size >= size()) return;
    if (new_size == 0) {
      clear();
      return;
    }

    while (size() > new_size) {
      _erase_lru();
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  // SIZE AND CAPACITY INTERFACE
  /////////////////////////////////////////////////////////////////////////////

  /// \returns The number of keys present in the cache.
  virtual size_t size() const noexcept {
    return _map.size();
  }

  /// Sets the capacity of the cache to the given value.
  ///
  /// If the given capacity is less than the current capacity of the cache,
  /// the least-recently inserted element is removed repeatedly until the
  /// capacity is equal to the given value.
  ///
  /// \param new_capacity The capacity to shrink or grow to.
  virtual void capacity(size_t new_capacity) {
    // Pop the front of the cache if we have to resize
    while (size() > new_capacity) {
      _erase_lru();
    }
    _capacity = new_capacity;
  }

  /// Returns the current capacity of the cache.
  virtual size_t capacity() const noexcept {
    return _capacity;
  }

  /// \returns the number of slots left in the cache.
  ///
  /// \details After this number of elements have been inserted, the next one
  /// insertion is preceded by an erasure of the least-recently inserted
  /// element.
  virtual size_t space_left() const noexcept {
    return _capacity - size();
  }

  /// \returns True if the cache contains no elements, else false.
  virtual bool is_empty() const noexcept {
    return size() == 0;
  }

  /// \returns True if the cache's size equals its capacity, else false.
  ///
  /// \details If `is_full()` returns `true`, the next insertion is preceded by
  /// an erasure of the least-recently inserted element.
  virtual bool is_full() const noexcept {
    return size() == _capacity;
  }

  /// \returns The function used to hash keys.
  virtual HashFunction hash_function() const {
    return _map.hash_function();
  }

  /// \returns The function used to compare keys.
  virtual KeyEqual key_equal() const {
    return _map.key_eq();
  }

  /////////////////////////////////////////////////////////////////////////////
  // STATISTICS INTERFACE
  /////////////////////////////////////////////////////////////////////////////

  /// Registers the given statistics object for monitoring.
  ///
  /// This method is useful if the statistics object is to
  /// be shared between caches.
  ///
  /// Ownership of the statistics object remains with the user and __not__ with
  /// the cache object. Also, behavior is undefined if the lifetime of the cache
  /// exceeds that of the registered statistics object.
  ///
  /// \param statistics The statistics object to register.
  virtual void monitor(const StatisticsPointer& statistics) {
    _stats = statistics;
  }

  /// Registers the given statistics object for monitoring.
  ///
  /// Ownership of the statistics object is transferred to the cache.
  ///
  /// \param statistics The statistics object to register.
  virtual void monitor(StatisticsPointer&& statistics) {
    _stats = std::move(statistics);
  }

  /// Constructs a new statistics in-place in the cache.
  ///
  /// This method is useful if the cache is to have exclusive ownership of the
  /// statistics and out-of-place construction and move is inconvenient.
  ///
  /// \param args Arguments to be forwarded to the constructor of the statistics
  ///             object.
  template <typename... Args,
            typename = std::enable_if_t<
                Internal::none_of_type<StatisticsPointer, Args...>>>
  void monitor(Args&&... args) {
    _stats = std::make_shared<Statistics<Key>>(std::forward<Args>(args)...);
  }

  /// Stops any monitoring being performed with a statistics object.
  ///
  /// If the cache is not currently monitoring at all, this is a no-op.
  virtual void stop_monitoring() {
    _stats.reset();
  }

  /// \returns True if the cache is currently monitoring statistics, else
  /// false.
  bool is_monitoring() const noexcept {
    return _stats.has_stats();
  }

  /// \returns The statistics object currently in use by the cache.
  /// \throws LRU::Error::NotMonitoring if the cache is currently not
  /// monitoring.
  virtual Statistics<Key>& stats() {
    if (!is_monitoring()) {
      throw LRU::Error::NotMonitoring();
    }
    return _stats.get();
  }

  /// \returns The statistics object currently in use by the cache.
  /// \throws LRU::Error::NotMonitoring if the cache is currently not
  /// monitoring.
  virtual const Statistics<Key>& stats() const {
    if (!is_monitoring()) {
      throw LRU::Error::NotMonitoring();
    }
    return _stats.get();
  }

  /// \returns A `shared_ptr` to the statistics currently in use by the cache.
  virtual StatisticsPointer& shared_stats() {
    return _stats.shared();
  }

  /// \returns A `shared_ptr` to the statistics currently in use by the cache.
  virtual const StatisticsPointer& shared_stats() const {
    return _stats.shared();
  }

 protected:
  using MapInsertionResult = decltype(Map().emplace());
  using LastAccessed =
      typename Internal::LastAccessed<Key, Information, KeyEqual>;

  /// Moves the key pointed to by the iterator to the front of the order.
  ///
  /// \param iterator The iterator pointing to the key to move.
  /// \param new_value The updated value to move the key with.
  virtual void _move_to_front(MapIterator iterator, const Value& new_value) {
    _order.erase(iterator->second.order);

    // Insert and get the iterator (push_back returns
    // void and emplace_back returns a reference ...)
    auto new_order = _order.insert(_order.end(), iterator->first);

    iterator->second.order = new_order;
    iterator->second.value = new_value;

    _last_accessed = iterator;
  }

  /// Erases the element most recently inserted into the cache.
  virtual void _erase_lru() {
    _erase(_map.find(_order.front()));
  }

  /// Erases the element pointed to by the iterator.
  ///
  /// \param iterator The iterator pointing to the key to erase.
  virtual void _erase(MapConstIterator iterator) {
    if (_last_accessed == iterator) {
      _last_accessed.invalidate();
    }

    _order.erase(iterator->second.order);
    _map.erase(iterator);
  }

  /// Erases the given key.
  ///
  /// This method is useful if the key and information are already present, to
  /// avoid an additional hash lookup to get an iterator to the corresponding
  /// map entry.
  ///
  /// \param key The key to erase.
  /// \param information The information associated with the key to erase.
  virtual void _erase(const Key& key, const Information& information) {
    if (key == _last_accessed) {
      _last_accessed.invalidate();
    }

    _order.erase(information.order);

    // Requires an additional hash-lookup, whereas erase(iterator) doesn't
    _map.erase(key);
  }

  /// Convenience methhod to get the value for an insertion result into a map.
  /// \returns The value for the given result.
  virtual Value& _value_from_result(MapInsertionResult& result) noexcept {
    // `result.first` is the map iterator (to a pair), whose `second` member
    // is
    // the information object, whose `value` member is the value stored.
    return result.first->second.value;
  }

  /// The main use of this method is that it may be override by a base class
  /// if
  /// there are any stronger constraints (such as time expiration) as to when
  /// the last-accessed object may be used to access a key.
  ///
  /// \param key The key to compare the last accessed object against.
  /// \returns True if the last-accessed object is valid.
  virtual bool _last_accessed_is_ok(const Key& key) const noexcept {
    return true;
  }

  /// \copydoc _value_for_last_accessed() const
  virtual Value& _value_for_last_accessed() {
    return _last_accessed.value();
  }

  /// Attempts to access the last accessed key's value.
  /// \returns The value of the last accessed object.
  /// \details This method exists so that derived classes may perform
  /// additional
  /// checks (and possibly throw exceptions) or perform other operations to
  /// retrieve the value.
  virtual const Value& _value_for_last_accessed() const {
    return _last_accessed.value();
  }

  /// Registers a hit for the key, if the cache is currently monitoring.
  /// \param key The key to maybe register a hit for.
  virtual void _register_hit_if_monitoring(const Key& key) const {
    if (is_monitoring()) {
      _stats.register_hit(key);
    }
  }

  /// Registers a miss for the key, if the cache is currently monitoring.
  /// \param key The key to maybe register a miss for.
  virtual void _register_miss_if_monitoring(const Key& key) const {
    if (is_monitoring()) {
      _stats.register_miss(key);
    }
  }

  /// The common part of both range assignment operators.
  ///
  /// \param range The range to assign to.
  template <typename Range>
  void _clear_and_increase_capacity(const Range& range) {
    using std::begin;
    using std::end;

    clear();

    auto distance = std::distance(begin(range), end(range));
    if (distance > _capacity) {
      _capacity = distance;
    }
  }

  /// The map from keys to information objects.
  Map _map;

  /// The queue keeping track of the insertion order of elements.
  Queue _order;

  /// The object to mutate statistics if any are registered.
  mutable StatisticsMutator<Key> _stats;

  /// The last-accessed cache object.
  mutable LastAccessed _last_accessed;

  /// The current capacity of the cache.
  size_t _capacity;
};
}  // namespace Internal
}  // namespace LRU

#endif  // LRU_INTERNAL_BASE_CACHE_HPP
