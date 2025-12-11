// Google Benchmark-based micro-benchmarks for LRU Cache variants

#include <cstddef>
#include <thread>
#include <vector>

#include <benchmark/benchmark.h>

#include "concurrent_lru_cache_serialized.hpp"

template <template <typename, typename> typename CacheType, typename Key = int,
          typename Value = int>
static void ReadOnly(benchmark::State &state) {
  size_t const capacity = state.range(0);
  size_t const num_readers = state.range(1);
  size_t const num_total_gets = state.range(2);
  size_t const num_gets_per_reader = num_total_gets / num_readers;
  size_t num_items_processed = 0;

  auto cache = CacheType<Key, Value>(capacity);
  for (int i = 0; i < capacity; ++i) {
    cache.Put(i, i);
  }

  for (auto _ : state) {
    auto readers = std::vector<std::jthread>{};
    readers.reserve(num_readers);
    for (size_t i = 0; i < num_readers; ++i) {
      readers.emplace_back(
          *[](CacheType<Key, Value> *cache, size_t num_gets) {
            auto const modulus = static_cast<Key>(cache->Capacity() * 1.5);
            for (size_t j = 0; j < num_gets; ++j) {
              auto key = j % modulus;
              benchmark::DoNotOptimize(cache->Get(key));
            }
          },
          &cache, num_gets_per_reader);
    }
    num_items_processed += num_total_gets;
  }

  state.SetItemsProcessed(num_items_processed);
}

static void
BM_ConcurrentLRUCacheSerializedMemoryOptimized(benchmark::State &state) {
  ReadOnly<ConcurrentLRUCacheSerializedMemoryOptimized>(state);
}
BENCHMARK(BM_ConcurrentLRUCacheSerializedMemoryOptimized)
    ->Args({1'000, 1, 1'000'000})
    ->Args({10'000, 1, 1'000'000})
    ->Args({100'000, 1, 1'000'000})
    ->Args({1'000, 10, 1'000'000})
    ->Args({10'000, 10, 1'000'000})
    ->Args({100'000, 10, 1'000'000});

static void BM_ConcurrentLRUCacheSerializedList(benchmark::State &state) {
  ReadOnly<ConcurrentLRUCacheSerializedList>(state);
}
BENCHMARK(BM_ConcurrentLRUCacheSerializedList)
    ->Args({1'000, 1, 1'000'000})
    ->Args({10'000, 1, 1'000'000})
    ->Args({100'000, 1, 1'000'000})
    ->Args({1'000, 10, 1'000'000})
    ->Args({10'000, 10, 1'000'000})
    ->Args({100'000, 10, 1'000'000});

BENCHMARK_MAIN();
