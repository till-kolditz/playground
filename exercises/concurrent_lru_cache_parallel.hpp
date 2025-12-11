#pragma once

#include <cstddef>
#include <mutex>
#include <optional>
#include <shared_mutex>

#include "lru_cache.hpp"

template <template <typename K, typename V> typename BaseT, typename Key,
          typename Value>
class ConcurrentLRUCacheParallelRead : public BaseT<Key, Value> {
  using Base = BaseT<Key, Value>;

public:
  ConcurrentLRUCacheParallelRead(size_t capacity) : Base{capacity} {}

  void Put(const Key &key, const Value &value) override {
    auto lock = std::lock_guard{mtx};
    this->Base::Put(key, value);
  }

  std::optional<Value> Get(const Key &key) override {
    auto lock = std::shared_lock{mtx};
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

  /**
   * @brief Obtains a shared lock, so it may not be fully accurate during
   * parallel reads.
   */
  CacheStats GetStats() const override {
    auto lock = std::shared_lock{mtx};
    return this->Base::GetStats();
  }

  /**
   * @brief Obtains a shared lock, so it may not be fully accurate during
   * parallel reads.
   */
  CacheStats PutStats() const override {
    auto lock = std::shared_lock{mtx};
    return this->Base::PutStats();
  }

private:
  mutable std::shared_mutex mtx;
};

/**
 * @brief A thread-safe LRU Cache implementation allowing parallel reads using
 * just a single shared mutex. It is optimized for memory usage and simplicity
 * and uses a single hash map to store cache entries.
 */
template <typename Key = int, typename Value = int>
class ConcurrentLRUCacheParallelReadMemoryOptimized
    : public ConcurrentLRUCacheParallelRead<LRUCacheMemoryOptimized, Key,
                                            Value> {
  using Base =
      ConcurrentLRUCacheParallelRead<LRUCacheMemoryOptimized, Key, Value>;

public:
  using Base::Base;
};

/**
 * @brief A thread-safe LRU Cache implementation allowing parallel reads using
 * just a single shared mutex. It is optimized for latency/throughput by
 * maintaining a linked list to store the LRU order.
 */
template <typename Key = int, typename Value = int>
class ConcurrentLRUCacheParallelReadList
    : public ConcurrentLRUCacheParallelRead<LRUCacheListBased, Key, Value> {
  using Base = ConcurrentLRUCacheParallelRead<LRUCacheListBased, Key, Value>;

public:
  using Base::Base;

protected:
  void MoveToFront(typename Base::CacheEntry *iter) override {
    auto lock = std::lock_guard{list_mtx};
    Base::MoveToFront(iter);
  }

private:
  std::mutex list_mtx;
};
