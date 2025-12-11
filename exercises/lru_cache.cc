#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <iostream>
#include <stop_token>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

#include "concurrent_lru_cache_parallel.hpp"
#include "concurrent_lru_cache_serialized.hpp"
#include "lru_cache.hpp"

using std::string_view_literals::operator""sv;
using std::chrono_literals::operator""ns;
using std::chrono_literals::operator""s;

enum class OutputFormat { CSV, JSON };

constexpr OutputFormat kOutputFormat = OutputFormat::CSV;
constexpr auto kCsvFieldSeparator = ","sv;

struct Config {
  size_t op_modulo;
  size_t num_threads;
  std::chrono::nanoseconds operation_delay;
};

template <template <typename Key, typename Value> typename CacheType,
          typename Key, typename Value>
std::chrono::nanoseconds MeasureThroughput(CacheType<Key, Value> *cache,
                                           Config const &reader_config,
                                           Config const &writer_config) {
  auto start = std::chrono::steady_clock::now();

  auto threads = std::vector<std::jthread>{};

  threads.reserve(reader_config.num_threads + writer_config.num_threads);
  for (int i = 0; i < reader_config.num_threads; ++i) {
    threads.emplace_back([cache, reader_config](std::stop_token token) {
      for (size_t op = 0; !token.stop_requested(); ++op) {
        cache->Get(op % reader_config.op_modulo);
        if (reader_config.operation_delay > 0ns) {
          std::this_thread::sleep_for(reader_config.operation_delay);
        }
      }
    });
  }

  for (int i = 0; i < writer_config.num_threads; ++i) {
    threads.emplace_back([cache, writer_config](std::stop_token token) {
      for (size_t op = 0; !token.stop_requested(); ++op) {
        cache->Put(op % writer_config.op_modulo, op);
        if (writer_config.operation_delay > 0ns) {
          std::this_thread::sleep_for(writer_config.operation_delay);
        }
      }
    });
  }

  std::this_thread::sleep_for(2s);
  for (auto &thread : threads) {
    thread.request_stop();
  }
  std::ranges::for_each(threads, [](auto &t) { t.join(); });

  auto end = std::chrono::steady_clock::now();
  return end - start;
}

template <typename Key, typename Value>
void PrintCacheStatsRecord(std::string_view cache_name,
                           LRUCache<Key, Value> const &cache,
                           const Config &reader_config,
                           const Config &writer_config,
                           std::chrono::nanoseconds duration,
                           OutputFormat output_format) {
  auto get_stats = cache.GetStats();
  auto put_stats = cache.PutStats();
  const size_t num_reads = get_stats.hits + get_stats.misses;
  const size_t num_writes = put_stats.hits + put_stats.misses;
  const auto dur_ms =
      std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
  const auto throughput = static_cast<size_t>(
      static_cast<double>(num_reads + num_writes) - (dur_ms / 1000.0));

  if (output_format == OutputFormat::CSV) {
    std::cout << cache_name << kCsvFieldSeparator << cache.Capacity()
              << kCsvFieldSeparator << reader_config.num_threads
              << kCsvFieldSeparator << num_reads << kCsvFieldSeparator
              << writer_config.num_threads << kCsvFieldSeparator << num_writes
              << kCsvFieldSeparator << dur_ms << kCsvFieldSeparator
              << get_stats.hits << kCsvFieldSeparator << get_stats.misses
              << kCsvFieldSeparator << put_stats.hits << kCsvFieldSeparator
              << put_stats.misses << kCsvFieldSeparator << throughput;
  } else if (output_format == OutputFormat::JSON) {
    std::cout << '{' << R"("name": ")" << cache_name << R"(", "capacity":)"
              << cache.Capacity() << R"(, "num_readers": )"
              << reader_config.num_threads << R"(, "total_reads": )"
              << num_reads << R"(, "num_writers": )"
              << writer_config.num_threads << R"(, "total_writes": )"
              << num_writes << R"(, "duration_ms": )" << dur_ms
              << R"(, "read_hits": )" << get_stats.hits
              << R"(, "read_misses": )" << get_stats.misses
              << R"(, "write_hits": )" << put_stats.hits
              << R"(, "write_misses": )" << put_stats.misses
              << R"(, "throughput_ops_sec": )" << throughput;
  }
}

template <template <typename Key, typename Value> typename CacheType,
          typename Key = int, typename Value = int>
void PreFillCache(CacheType<Key, Value> *cache) {
  for (Key k = 0; k < static_cast<int>(cache->Capacity()); ++k) {
    cache->Put(k, k);
  }
}

template <template <typename Key, typename Value> typename CacheType,
          typename Key = int, typename Value = int>
void RunCacheBenchmark(std::string_view cache_name, size_t cache_capacity,
                       Config const &reader_config, Config const &writer_config,
                       OutputFormat output_format) {
  auto cache = CacheType<Key, Value>{cache_capacity};

  PreFillCache(&cache);

  auto duration = MeasureThroughput(&cache, reader_config, writer_config);

  PrintCacheStatsRecord(cache_name, cache, reader_config, writer_config,
                        duration, output_format);
}

void PrintOutputPreRecord() {
  if (kOutputFormat == OutputFormat::JSON) {
    std::cout << "\t";
  }
}

void PrintOutputRecordSeparator() {
  if (kOutputFormat == OutputFormat::CSV) {
    std::cout << "\n";
  } else if (kOutputFormat == OutputFormat::JSON) {
    std::cout << ",\n";
  }
}

template <template <typename Key, typename Value> typename CacheType,
          typename Key = int, typename Value = int>
void RunCacheBenchmarkCases(std::string_view cache_name,
                            OutputFormat output_format) {
  for (auto const cache_capacity :
       std::array<size_t, 5>{100, 1'000, 10'000, 100'000, 1'000'000}) {
    // first reader config then writer config
    const auto read_modulos = static_cast<size_t>(cache_capacity * 1.5);
    const auto write_modulos = static_cast<size_t>(cache_capacity * 1.1);
    for (auto const &configs : std::array<std::pair<Config, Config>, 8>{
             {{{read_modulos, 1, 0ns}, {write_modulos, 0, 0ns}},
              {{read_modulos, 10, 0ns}, {write_modulos, 0, 0ns}},
              {{read_modulos, 100, 0ns}, {write_modulos, 0, 0ns}},
              {{read_modulos, 0, 0ns}, {write_modulos, 1, 0ns}},
              {{read_modulos, 0, 0ns}, {write_modulos, 10, 0ns}},
              {{read_modulos, 0, 0ns}, {write_modulos, 100, 0ns}},
              {{read_modulos, 10, 0ns}, {write_modulos, 2, 1'000ns}},
              {{read_modulos, 2, 0ns}, {write_modulos, 10, 1'000ns}}}}) {
      auto const &reader_config = configs.first;
      auto const &writer_config = configs.second;

      PrintOutputPreRecord();
      RunCacheBenchmark<CacheType>(cache_name, cache_capacity, reader_config,
                                   writer_config, output_format);
      PrintOutputRecordSeparator();
    }
  }
}

void WriteOutputHeader() {
  if (kOutputFormat == OutputFormat::CSV) {
    std::cout << "name" << kCsvFieldSeparator << "capacity"
              << kCsvFieldSeparator << "num_readers" << kCsvFieldSeparator
              << "total_reads" << kCsvFieldSeparator << "num_writers"
              << kCsvFieldSeparator << "total_writes" << kCsvFieldSeparator
              << "duration_ms" << kCsvFieldSeparator << "read_hits"
              << kCsvFieldSeparator << "read_misses" << kCsvFieldSeparator
              << "write_hits" << kCsvFieldSeparator << "write_misses"
              << kCsvFieldSeparator << "throughput_ops_sec\n";
  } else if (kOutputFormat == OutputFormat::JSON) {
    std::cout << "[\n";
  }
}

void WriteOutputFooter() {
  if (kOutputFormat == OutputFormat::JSON) {
    std::cout << "]\n";
  }
}

int main() {
  WriteOutputHeader();

  RunCacheBenchmarkCases<ConcurrentLRUCacheSerializedMemoryOptimized>(
      "ConcurrentLRUCacheSerializedMemoryOptimized", kOutputFormat);
  RunCacheBenchmarkCases<ConcurrentLRUCacheSerializedList>(
      "ConcurrentLRUCacheSerializedList", kOutputFormat);
  RunCacheBenchmarkCases<ConcurrentLRUCacheParallelReadMemoryOptimized>(
      "ConcurrentLRUCacheParallelReadMemoryOptimized", kOutputFormat);
  RunCacheBenchmarkCases<ConcurrentLRUCacheParallelReadList>(
      "ConcurrentLRUCacheParallelReadList", kOutputFormat);

  WriteOutputFooter();
  return 0;
}
