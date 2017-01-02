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

#ifndef LRU_INTERNAL_CALLBACK_MANAGER_HPP
#define LRU_INTERNAL_CALLBACK_MANAGER_HPP

#include <functional>
#include <vector>

#include "lru/entry.hpp"
#include "lru/internal/optional.hpp"

namespace LRU {
namespace Internal {

template <typename Key, typename Value>
class CallbackManager {
 public:
  using HitCallback = std::function<void(const Key&, const Value&)>;
  using MissCallback = std::function<void(const Key&)>;
  using AccessCallback = std::function<void(const Key&, bool)>;

  using HitCallbackContainer = std::vector<HitCallback>;
  using MissCallbackContainer = std::vector<MissCallback>;
  using AccessCallbackContainer = std::vector<AccessCallback>;

  void hit(const Key& key, const Value& value) {
    _call_each(_hit_callbacks, key, value);
    _call_each(_access_callbacks, key, true);
  }

  void miss(const Key& key) {
    _call_each(_miss_callbacks, key);
    _call_each(_access_callbacks, key, false);
  }

  template <typename Callback>
  void hit_callback(Callback&& hit_callback) {
    _hit_callbacks.emplace_back(std::forward<Callback>(hit_callback));
  }

  template <typename Callback>
  void miss_callback(Callback&& miss_callback) {
    _miss_callbacks.emplace_back(std::forward<Callback>(miss_callback));
  }

  template <typename Callback>
  void access_callback(Callback&& access_callback) {
    _access_callbacks.emplace_back(std::forward<Callback>(access_callback));
  }

  void clear_hit_callbacks() {
    _hit_callbacks.clear();
  }

  void clear_miss_callbacks() {
    _miss_callbacks.clear();
  }

  void clear_access_callbacks() {
    _access_callbacks.clear();
  }

  void clear() {
    clear_hit_callbacks();
    clear_miss_callbacks();
    clear_access_callbacks();
  }

  const HitCallbackContainer& hit_callbacks() const noexcept {
    return _hit_callbacks;
  }

  const MissCallbackContainer& miss_callbacks() const noexcept {
    return _miss_callbacks;
  }

  const AccessCallbackContainer& access_callbacks() const noexcept {
    return _access_callbacks;
  }

 private:
  template <typename CallbackContainer, typename... Args>
  void _call_each(const CallbackContainer& callbacks, Args&&... args) {
    for (const auto& callback : callbacks) {
      callback(std::forward<Args>(args)...);
    }
  }


  HitCallbackContainer _hit_callbacks;
  MissCallbackContainer _miss_callbacks;
  AccessCallbackContainer _access_callbacks;
};
}  // namespace Internal
}  // namespace LRU

#endif  // LRU_INTERNAL_CALLBACK_MANAGER_HPP
