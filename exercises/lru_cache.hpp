#pragma once

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <list>
#include <optional>
#include <unordered_map>
#include <utility>

struct CacheStats {
  size_t hits = 0;
  size_t misses = 0;
};

template <typename Key = int, typename Value = int> class LRUCache {
public:
  LRUCache(size_t capacity) : capacity{capacity} {}

  virtual void Put(const Key &key, const Value &value) = 0;
  virtual std::optional<Value> Get(const Key &key) = 0;
  virtual size_t Capacity() const { return capacity; }
  virtual CacheStats GetStats() const { return get_stats; };
  virtual CacheStats PutStats() const { return put_stats; };
  virtual void ClearCacheAndResetStats() = 0;
  virtual void Resize(size_t new_capacity) = 0;

protected:
  size_t capacity;
  CacheStats get_stats{};
  CacheStats put_stats{};
};

/**
 * @brief A thread-unsafe LRU Cache implementation with serialized access using
 * just a single mutex. It is optimized for latency/throughput by maintaining a
 * linked list to store the LRU order.
 */
template <typename Key = int, typename Value = int>
class LRUCacheListBased : public LRUCache<Key, Value> {
  using Base = LRUCache<Key, Value>;

protected:
  using LruList = std::list<std::pair<Key, Value>>;
  using CacheEntry = typename LruList::iterator;

public:
  LRUCacheListBased(size_t capacity) : Base{capacity} {
    cache.reserve(capacity);
  }

  void Put(const Key &key, const Value &value) override {
    assert(cache.size() <= this->Base::capacity);

    if (auto iter = cache.find(key); iter != cache.end()) {
      MoveToFront(&iter->second);
      iter->second->second = value;
      ++this->Base::put_stats.hits;
    } else {
      assert(cache.size() <= this->Base::capacity);

      if (cache.size() >= this->Base::capacity) {
        auto iter = std::prev(std::end(lru_list));
        MoveToFront(&iter);
        cache.erase(iter->first);
        iter->first = key;
        iter->second = value;
        assert(cache.size() == this->Base::capacity - 1);
        assert(iter == lru_list.begin());
        cache.emplace(key, iter);
      } else {
        lru_list.push_front(std::make_pair(key, value));
        cache.emplace(key, lru_list.begin());
      }
      ++this->Base::put_stats.misses;
    }

    assert(cache.size() <= this->Base::capacity);
  }

  std::optional<Value> Get(const Key &key) override {
    assert(cache.size() <= this->Base::capacity);

    if (auto iter = cache.find(key); iter != cache.end()) {
      ++this->Base::get_stats.hits;
      MoveToFront(&iter->second);
      return (*iter->second).second;
    } else {
      ++this->Base::get_stats.misses;
      return std::nullopt;
    }
  }

  void ClearCacheAndResetStats() override {
    cache.clear();
    this->Base::get_stats = CacheStats{};
    this->Base::put_stats = CacheStats{};
    lru_list.clear();
  }

  void Resize(size_t new_capacity) override {
    if (new_capacity >= this->Base::capacity) {
      cache.reserve(new_capacity);
    } else {
      while (cache.size() > new_capacity) {
        auto iter = std::prev(std::end(lru_list));
        cache.erase(iter->first);
        lru_list.pop_back();
      }
    }
    this->Base::capacity = new_capacity;
  }

protected:
  virtual void MoveToFront(CacheEntry *iter) {
    lru_list.splice(lru_list.begin(), lru_list, *iter);
  }

  LruList lru_list;
  std::unordered_map<Key, CacheEntry> cache;
};
/**
 * @brief A thread-unsafe LRU Cache implementation with serialized access using
 * just a single mutex. It is optimized for memory usage and simplicity and uses
 * a single hash map to store cache entries.
 */
template <typename Key = int, typename Value = int>
class LRUCacheMemoryOptimized : public LRUCache<Key, Value> {
  using Base = LRUCache<Key, Value>;

  struct Entry {
    Value value;
    size_t latest_access_ts;
  };

  void RemoveLeastRecentlyUsed() {
    auto lru_iter = std::min_element(
        cache.begin(), cache.end(), [](const auto &a, const auto &b) {
          return a.second.latest_access_ts < b.second.latest_access_ts;
        });
    cache.erase(lru_iter);
  }

public:
  LRUCacheMemoryOptimized(size_t capacity) : Base{capacity} {
    cache.reserve(capacity);
  }

  void Put(const Key &key, const Value &value) override {
    assert(cache.size() <= this->Base::capacity);

    if (auto iter = cache.find(key); iter != cache.end()) {
      iter->second.value = value;
      iter->second.latest_access_ts = ++current_ts;
      ++this->Base::put_stats.hits;
    } else {
      if (cache.size() >= this->Base::capacity) {
        RemoveLeastRecentlyUsed();
      }
      cache.emplace(key, Entry{value, ++current_ts});
      ++this->Base::put_stats.misses;
    }

    assert(cache.size() <= this->Base::capacity);
  }

  std::optional<Value> Get(const Key &key) override {
    assert(cache.size() <= this->Base::capacity);

    if (auto iter = cache.find(key); iter != cache.end()) {
      ++this->Base::get_stats.hits;
      iter->second.latest_access_ts = ++current_ts;
      return iter->second.value;
    } else {
      ++this->Base::get_stats.misses;
      return std::nullopt;
    }
  }

  void ClearCacheAndResetStats() override {
    cache.clear();
    this->Base::get_stats = CacheStats{};
    this->Base::put_stats = CacheStats{};
    current_ts = 0;
  }

  void Resize(size_t new_capacity) override {
    if (new_capacity >= this->Base::capacity) {
      cache.reserve(new_capacity);
    } else {
      while (cache.size() > new_capacity) {
        RemoveLeastRecentlyUsed();
      }
    }
    this->Base::capacity = new_capacity;
  }

private:
  std::atomic<size_t> current_ts{0};
  std::unordered_map<Key, Entry> cache;
};
