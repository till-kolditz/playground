#pragma once

#include <cassert>
#include <cstddef>
#include <mutex>
#include <optional>

#include "lru_cache.hpp"

template <template <typename K, typename V> typename BaseT, typename Key,
          typename Value>
class ConcurrentLRUCacheSerialized : public BaseT<Key, Value> {
  using Base = BaseT<Key, Value>;

public:
  ConcurrentLRUCacheSerialized(size_t capacity) : Base{capacity} {}

  void Put(const Key &key, const Value &value) override {
    auto lock = std::lock_guard{mtx};
    this->Base::Put(key, value);
  }

  std::optional<Value> Get(const Key &key) override {
    auto lock = std::lock_guard{mtx};
    return this->Base::Get(key);
  }

  void ClearCacheAndResetStats() override {
    auto lock = std::lock_guard{mtx};
    this->Base::ClearCacheAndResetStats();
  }

  void Resize(size_t new_capacity) override {
    auto lock = std::lock_guard{mtx};
    this->Base::Resize(new_capacity);
  }

  CacheStats GetStats() const override {
    auto lock = std::lock_guard{mtx};
    return this->Base::GetStats();
  }

  CacheStats PutStats() const override {
    auto lock = std::lock_guard{mtx};
    return this->Base::PutStats();
  }

private:
  mutable std::mutex mtx;
};
/**
 * @brief A thread-safe LRU Cache implementation with serialized access using
 * just a single mutex. It is optimized for memory usage and simplicity and uses
 * a single hash map to store cache entries.
 */
template <typename Key = int, typename Value = int>
class ConcurrentLRUCacheSerializedMemoryOptimized
    : public ConcurrentLRUCacheSerialized<LRUCacheMemoryOptimized, Key, Value> {
  using Base =
      ConcurrentLRUCacheSerialized<LRUCacheMemoryOptimized, Key, Value>;

public:
  using Base::Base;
};

/**
 * @brief A thread-safe LRU Cache implementation with serialized access using
 * just a single mutex. It is optimized for latency/throughput by maintaining a
 * linked list to store the LRU order.
 */
template <typename Key = int, typename Value = int>
class ConcurrentLRUCacheSerializedList
    : public ConcurrentLRUCacheSerialized<LRUCacheListBased, Key, Value> {
  using Base = ConcurrentLRUCacheSerialized<LRUCacheListBased, Key, Value>;

public:
  using Base::Base;
};
