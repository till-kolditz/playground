#include <thread>

#include <gtest/gtest.h>

#include "concurrent_lru_cache_parallel.hpp"
#include "concurrent_lru_cache_serialized.hpp"

TEST(ConcurrentLRUCacheSerializedMemoryOptimized, EmptyCache) {
  auto cache = ConcurrentLRUCacheSerializedMemoryOptimized<int, int>{3};

  // Test Get from empty cache
  EXPECT_EQ(cache.Get(1), std::nullopt);
}

TEST(ConcurrentLRUCacheSerializedMemoryOptimized, BasicOperations) {
  auto cache = ConcurrentLRUCacheSerializedMemoryOptimized<int, int>{3};

  // Test Put and Get
  cache.Put(1, 10);
  cache.Put(2, 20);
  cache.Put(3, 30);

  EXPECT_EQ(cache.Get(1), 10);
  EXPECT_EQ(cache.Get(2), 20);
  EXPECT_EQ(cache.Get(3), 30);

  // Test LRU eviction
  cache.Put(4, 40);
  EXPECT_EQ(cache.Get(1), std::nullopt); // 1 should be evicted
  EXPECT_EQ(cache.Get(4), 40);
}

TEST(ConcurrentLRUCacheSerializedMemoryOptimized, Concurrency) {
  auto cache = ConcurrentLRUCacheSerializedMemoryOptimized<int, int>{100};

  auto writer = [&cache]() {
    for (int i = 0; i < 1000; ++i) {
      cache.Put(i, i * 10);
      EXPECT_EQ(cache.Get(i), i * 10);
    }
  };

  auto reader = [&cache]() {
    for (int i = 0; i < 1000; ++i) {
      auto opt = cache.Get(i);
      if (opt) {
        EXPECT_EQ(*opt, i * 10);
      }
    }
  };

  std::jthread writer_thread(writer);
  std::jthread reader_thread(reader);
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  writer_thread.join();
  reader_thread.join();

  for (int i = 900; i < 1000; ++i) {
    EXPECT_EQ(cache.Get(i), i * 10);
  }
}

TEST(ConcurrentLRUCacheSerializedList, EmptyCache) {
  auto cache = ConcurrentLRUCacheSerializedList<int, int>{3};

  // Test Get from empty cache
  EXPECT_EQ(cache.Get(1), std::nullopt);
}

TEST(ConcurrentLRUCacheSerializedList, BasicOperations) {
  auto cache = ConcurrentLRUCacheSerializedList<int, int>{3};

  // Test Put and Get
  cache.Put(1, 10);
  cache.Put(2, 20);
  cache.Put(3, 30);

  EXPECT_EQ(cache.Get(1), 10);
  EXPECT_EQ(cache.Get(2), 20);
  EXPECT_EQ(cache.Get(3), 30);

  // Test LRU eviction
  cache.Put(4, 40);
  EXPECT_EQ(cache.Get(1), std::nullopt); // 1 should be evicted
  EXPECT_EQ(cache.Get(4), 40);
}

TEST(ConcurrentLRUCacheSerializedList, Concurrency) {
  auto cache = ConcurrentLRUCacheSerializedList<int, int>{100};

  auto writer = [&cache]() {
    for (int i = 0; i < 1000; ++i) {
      cache.Put(i, i * 10);
      EXPECT_EQ(cache.Get(i), i * 10);
    }
  };

  auto reader = [&cache]() {
    for (int i = 0; i < 1000; ++i) {
      auto opt = cache.Get(i);
      if (opt) {
        EXPECT_EQ(*opt, i * 10);
      }
    }
  };

  std::jthread writer_thread(writer);
  std::jthread reader_thread(reader);
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  writer_thread.join();
  reader_thread.join();

  for (int i = 900; i < 1000; ++i) {
    EXPECT_EQ(cache.Get(i), i * 10);
  }
}

TEST(ConcurrentLRUCacheParallelReadMemoryOptimized, EmptyCache) {
  auto cache = ConcurrentLRUCacheParallelReadMemoryOptimized<int, int>{3};

  // Test Get from empty cache
  EXPECT_EQ(cache.Get(1), std::nullopt);
}

TEST(ConcurrentLRUCacheParallelReadMemoryOptimized, BasicOperations) {
  auto cache = ConcurrentLRUCacheParallelReadMemoryOptimized<int, int>{3};

  // Test Put and Get
  cache.Put(1, 10);
  cache.Put(2, 20);
  cache.Put(3, 30);

  EXPECT_EQ(cache.Get(1), 10);
  EXPECT_EQ(cache.Get(2), 20);
  EXPECT_EQ(cache.Get(3), 30);

  // Test LRU eviction
  cache.Put(4, 40);
  EXPECT_EQ(cache.Get(1), std::nullopt); // 1 should be evicted
  EXPECT_EQ(cache.Get(4), 40);
}

TEST(ConcurrentLRUCacheParallelReadMemoryOptimized, Concurrency) {
  auto cache = ConcurrentLRUCacheParallelReadMemoryOptimized<int, int>{100};

  auto writer = [&cache]() {
    for (int i = 0; i < 1000; ++i) {
      cache.Put(i, i * 10);
      EXPECT_EQ(cache.Get(i), i * 10);
    }
  };

  auto reader = [&cache]() {
    for (int i = 0; i < 1000; ++i) {
      auto opt = cache.Get(i);
      if (opt) {
        EXPECT_EQ(*opt, i * 10);
      }
    }
  };

  std::jthread writer_thread(writer);
  std::jthread reader_thread(reader);
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  writer_thread.join();
  reader_thread.join();

  for (int i = 900; i < 1000; ++i) {
    EXPECT_EQ(cache.Get(i), i * 10);
  }
}

TEST(ConcurrentLRUCacheParallelReadList, EmptyCache) {
  auto cache = ConcurrentLRUCacheParallelReadList<int, int>{3};

  // Test Get from empty cache
  EXPECT_EQ(cache.Get(1), std::nullopt);
}

TEST(ConcurrentLRUCacheParallelReadList, BasicOperations) {
  auto cache = ConcurrentLRUCacheParallelReadList<int, int>{3};

  // Test Put and Get
  cache.Put(1, 10);
  cache.Put(2, 20);
  cache.Put(3, 30);

  EXPECT_EQ(cache.Get(1), 10);
  EXPECT_EQ(cache.Get(2), 20);
  EXPECT_EQ(cache.Get(3), 30);

  // Test LRU eviction
  cache.Put(4, 40);
  EXPECT_EQ(cache.Get(1), std::nullopt); // 1 should be evicted
  EXPECT_EQ(cache.Get(4), 40);
}

TEST(ConcurrentLRUCacheParallelReadList, Concurrency) {
  auto cache = ConcurrentLRUCacheParallelReadList<int, int>{100};

  auto writer = [&cache]() {
    for (int i = 0; i < 1000; ++i) {
      cache.Put(i, i * 10);
      EXPECT_EQ(cache.Get(i), i * 10);
    }
  };

  auto reader = [&cache]() {
    for (int i = 0; i < 1000; ++i) {
      auto opt = cache.Get(i);
      if (opt) {
        EXPECT_EQ(*opt, i * 10);
      }
    }
  };

  std::jthread writer_thread(writer);
  std::jthread reader_thread(reader);
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  writer_thread.join();
  reader_thread.join();

  for (int i = 900; i < 1000; ++i) {
    EXPECT_EQ(cache.Get(i), i * 10);
  }
}
