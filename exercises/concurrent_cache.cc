#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <iostream>
#include <mutex>
#include <optional>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

using std::chrono_literals::operator""ns;

struct CacheStats {
  size_t hits = 0;
  size_t misses = 0;
};

class ConcurrentLRUCache {
public:
  virtual void Put(int key, int value) = 0;
  virtual std::optional<int> Get(int key) = 0;
  virtual size_t Capacity() const = 0;
  virtual CacheStats GetStats() const = 0;
  virtual CacheStats PutStats() const = 0;
};

class ConcurrentLRUCacheSingleReaderSingleWriter : public ConcurrentLRUCache {
  struct Entry {
    int value;
    size_t latest_access_ts;
  };

public:
  ConcurrentLRUCacheSingleReaderSingleWriter(size_t capacity)
      : ConcurrentLRUCache{}, capacity{capacity} {
    cache.reserve(capacity);
  }

  void Put(int key, int value) override {

    auto lock = std::lock_guard{mtx};
    assert(cache.size() <= capacity);

    if (auto iter = cache.find(key); iter != cache.end()) {
      iter->second.value = value;
      iter->second.latest_access_ts = ++current_ts;
      ++put_stats.hits;
    } else {
      if (cache.size() >= capacity) {
        auto lru_iter = std::min_element(
            cache.begin(), cache.end(), [](const auto &a, const auto &b) {
              return a.second.latest_access_ts < b.second.latest_access_ts;
            });
        cache.erase(lru_iter);
      }
      cache.emplace(key, Entry{value, ++current_ts});
      ++put_stats.misses;
    }
  }

  std::optional<int> Get(int key) override {
    auto lock = std::lock_guard{mtx};
    assert(cache.size() <= capacity);

    if (auto iter = cache.find(key); iter != cache.end()) {
      ++get_stats.hits;
      iter->second.latest_access_ts = ++current_ts;
      return iter->second.value;
    } else {
      ++get_stats.misses;
      return std::nullopt;
    }
  }

  size_t Capacity() const override { return capacity; }
  CacheStats GetStats() const override { return get_stats; }
  CacheStats PutStats() const override { return put_stats; }

private:
  size_t capacity;
  CacheStats get_stats{};
  CacheStats put_stats{};
  std::atomic<size_t> current_ts{0};
  std::unordered_map<int, Entry> cache;
  std::mutex mtx;
};

struct Config {
  size_t op_modulo;
  size_t num_threads;
  size_t num_operations_per_thread;
  std::chrono::nanoseconds operation_delay;
};

std::chrono::nanoseconds MeasureThroughput(ConcurrentLRUCache *cache,
                                           Config const &reader_config,
                                           Config const &writer_config) {
  auto start = std::chrono::steady_clock::now();

  auto threads = std::vector<std::jthread>{};

  threads.reserve(reader_config.num_threads + writer_config.num_threads);
  for (int i = 0; i < reader_config.num_threads; ++i) {
    threads.emplace_back([cache, reader_config]() {
      for (int op = 0; op < reader_config.num_operations_per_thread; ++op) {
        cache->Get(op % reader_config.op_modulo);
        std::this_thread::sleep_for(reader_config.operation_delay);
      }
    });
  }

  for (int i = 0; i < writer_config.num_threads; ++i) {
    threads.emplace_back([cache, writer_config]() {
      for (int op = 0; op < writer_config.num_operations_per_thread; ++op) {
        cache->Put(op % writer_config.op_modulo, op);
        std::this_thread::sleep_for(writer_config.operation_delay);
      }
    });
  }

  std::ranges::for_each(threads, [](auto &t) { t.join(); });

  auto end = std::chrono::steady_clock::now();
  return end - start;
}

void PrintCacheStats(ConcurrentLRUCache const &cache,
                     const Config &reader_config, const Config &writer_config,
                     std::chrono::nanoseconds duration) {
  const size_t num_reads =
      reader_config.num_threads * reader_config.num_operations_per_thread;
  const size_t num_writes =
      writer_config.num_threads * writer_config.num_operations_per_thread;
  const auto dur_ms =
      std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
  const auto throughput = static_cast<size_t>(
      static_cast<double>(num_reads + num_writes) / (dur_ms / 1000.0));
  std::cout << "Cache Capacity: " << cache.Capacity()
            << ", Readers: " << reader_config.num_threads
            << ", Total Reads: " << num_reads
            << ", Writers: " << writer_config.num_threads
            << ", Total Writes: " << num_writes << ", Duration: " << dur_ms
            << " ms"
            << ", Read Hits: " << cache.GetStats().hits
            << ", Read Misses:" << cache.GetStats().misses
            << ", Write Hits: " << cache.PutStats().hits
            << ", Write Misses: " << cache.PutStats().misses
            << ", Throughput: " << throughput << " ops/sec"
            << "\n";
}

int main() {
  for (auto const cache_capacity : std::array<size_t, 3>{100, 1000, 10000}) {
    // first reader config then writer config
    for (auto const &configs : std::array<std::pair<Config, Config>, 2>{
             {{{cache_capacity * 5, 10, 10000, 0ns},
               {cache_capacity * 2, 2, 5000, 0ns}},
              {{cache_capacity * 5, 10, 10000, 1000ns},
               {cache_capacity * 2, 2, 5000, 2000ns}}}}) {
      auto cache = ConcurrentLRUCacheSingleReaderSingleWriter{cache_capacity};
      // TODO: if needed, pre-populate the cache here

      auto const &reader_config = configs.first;
      auto const &writer_config = configs.second;

      auto duration = MeasureThroughput(&cache, reader_config, writer_config);

      PrintCacheStats(cache, reader_config, writer_config, duration);
    }
  }
  return 0;
}
