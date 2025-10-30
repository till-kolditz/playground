#include <random>
#include <vector>

#include <benchmark/benchmark.h>

#include "distance.hpp"

template <typename Func>
static void LargestDistanceBenchmark(benchmark::State &state, Func func) {
  std::vector<int> points;
  points.reserve(state.range(0));
  auto rnd = std::random_device{};
  auto eng = std::mt19937{rnd()};
  auto dist = std::uniform_int_distribution<int>(0, state.range(0) * 10);
  for (int i = 0; i < state.range(0); ++i) {
    points.push_back(dist(eng));
  }
  for (auto _ : state) {
    benchmark::DoNotOptimize(func(points));
  }
}

static void LargestDistanceNestedLoopBenchmark(benchmark::State &state) {
  LargestDistanceBenchmark(
      state, compute_largest_positive_distance_ordered_nested_loop);
}

static void LargestDistanceOptimizedBenchmark(benchmark::State &state) {
  LargestDistanceBenchmark(state,
                           compute_largest_positive_distance_ordered_optimized);
}

BENCHMARK(LargestDistanceNestedLoopBenchmark)
    ->Arg(1000)
    ->Arg(10000)
    ->Arg(100000);

BENCHMARK(LargestDistanceOptimizedBenchmark)
    ->Arg(1000)
    ->Arg(10000)
    ->Arg(100000);

BENCHMARK_MAIN();
