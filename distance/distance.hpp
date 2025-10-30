#pragma once

#include <algorithm>
#include <limits>
#include <span>

int compute_largest_positive_distance_ordered_nested_loop(
    std::span<const int> points) {
  int max_distance = 0;
  for (size_t i = 0; i < points.size(); ++i) {
    for (size_t j = i + 1; j < points.size(); ++j) {
      int distance = points[j] - points[i];
      if (distance > max_distance) {
        max_distance = distance;
      }
    }
  }
  return max_distance;
}

int compute_largest_positive_distance_ordered_optimized(
    std::span<const int> points) {
  if (points.empty()) {
    return std::numeric_limits<int>::max();
  }
  auto iter = points.begin();
  int smallest_min = *iter;
  int distance = 0;
  for (++iter; iter != points.end(); ++iter) {
    smallest_min = std::min(smallest_min, *iter);
    distance = std::max(distance, *iter - smallest_min);
  }
  return distance;
}
