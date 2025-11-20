#include <atomic>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <new>
#include <ratio>
#include <thread>

#ifdef __cpp_lib_hardware_interference_size
using std::hardware_constructive_interference_size;
using std::hardware_destructive_interference_size;
#else
// 64 bytes on x86-64 │ L1_CACHE_BYTES │ L1_CACHE_SHIFT │ __cacheline_aligned │
// ...
constexpr std::size_t hardware_constructive_interference_size = 64;
constexpr std::size_t hardware_destructive_interference_size = 64;
#endif

std::mutex cout_mutex;

constexpr int kMaxWriteIterations{10'000'000}; // the benchmark time tuning

// occupies one cache line
struct alignas(hardware_constructive_interference_size) {
  std::atomic_uint64_t x{};
  std::atomic_uint64_t y{};
} oneCacheLiner;

// occupies two cache lines
struct {
  alignas(hardware_destructive_interference_size) std::atomic_uint64_t x{};
  alignas(hardware_destructive_interference_size) std::atomic_uint64_t y{};
} twoCacheLiner;

inline auto now() noexcept { return std::chrono::high_resolution_clock::now(); }

template <bool xy> void oneCacheLinerThread() {
  auto const start = now();

  for (uint64_t count{}; count != kMaxWriteIterations; ++count)
    if constexpr (xy)
      oneCacheLiner.x.fetch_add(1, std::memory_order_relaxed);
    else
      oneCacheLiner.y.fetch_add(1, std::memory_order_relaxed);

  auto const elapsed = std::chrono::duration<double, std::milli>{now() - start};
  std::lock_guard lk{cout_mutex};
  std::cout << "oneCacheLinerThread() spent " << elapsed.count() << " ms\n";
  if constexpr (xy)
    oneCacheLiner.x = elapsed.count();
  else
    oneCacheLiner.y = elapsed.count();
}

template <bool xy> void twoCacheLinerThread() {
  const auto start{now()};

  for (uint64_t count{}; count != kMaxWriteIterations; ++count)
    if constexpr (xy)
      twoCacheLiner.x.fetch_add(1, std::memory_order_relaxed);
    else
      twoCacheLiner.y.fetch_add(1, std::memory_order_relaxed);

  auto const elapsed = std::chrono::duration<double, std::milli>{now() - start};
  std::lock_guard lk{cout_mutex};
  std::cout << "twoCacheLinerThread() spent " << elapsed.count() << " ms\n";
  if constexpr (xy)
    twoCacheLiner.x = elapsed.count();
  else
    twoCacheLiner.y = elapsed.count();
}

int main() {
  std::cout << "__cpp_lib_hardware_interference_size "
#ifdef __cpp_lib_hardware_interference_size
               "= "
            << __cpp_lib_hardware_interference_size << '\n';
#else
               "is not defined, use "
            << hardware_destructive_interference_size << " as fallback\n";
#endif

  std::cout << "hardware_destructive_interference_size == "
            << hardware_destructive_interference_size << '\n'
            << "hardware_constructive_interference_size == "
            << hardware_constructive_interference_size << "\n\n"
            << std::fixed << std::setprecision(2)
            << "sizeof( OneCacheLiner ) == " << sizeof(oneCacheLiner) << '\n'
            << "sizeof( TwoCacheLiner ) == " << sizeof(twoCacheLiner) << "\n\n";

  constexpr int kMaxRuns = 4;

  auto average1 = 0.0;
  for (auto i{0}; i != kMaxRuns; ++i) {
    {
      auto th1 = std::jthread{oneCacheLinerThread<false>};
      auto th2 = std::jthread{oneCacheLinerThread<true>};
    }
    average1 += oneCacheLiner.x + oneCacheLiner.y;
  }
  std::cout << "Average T1 time: " << (average1 / kMaxRuns / 2) << " ms\n\n";

  auto average2 = 0.0;
  for (auto i{0}; i != kMaxRuns; ++i) {
    {
      auto th1 = std::jthread{twoCacheLinerThread<false>};
      auto th2 = std::jthread{twoCacheLinerThread<true>};
    }
    average2 += twoCacheLiner.x + twoCacheLiner.y;
  }
  std::cout << "Average T2 time: " << (average2 / kMaxRuns / 2) << " ms\n\n"
            << "Ratio T1/T2:~ " << average1 / average2 << '\n';
}
