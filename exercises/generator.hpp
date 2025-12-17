#pragma once

#include <stdexcept>
#include <type_traits>

template <typename T> class NumberRange {
public:
  class iterator {
  public:
    iterator(T num, T step) : num{num}, step{step}, positive{step > T(0)} {
      if (step == T(0)) {
        throw std::invalid_argument{"step size 0 provided"};
      }
    }

    iterator &operator++() noexcept {
      num += step;
      return *this;
    }

    iterator &operator++(int) noexcept {
      auto copy = *this;
      num += step;
      return copy;
    }

    iterator &operator+=(int steps) noexcept {
      num += steps * step;
      return *this;
    }

    iterator &operator--() noexcept {
      num -= step;
      return *this;
    }

    iterator &operator--(int) noexcept {
      auto copy = *this;
      num -= step;
      return copy;
    }

    iterator &operator-=(int steps) noexcept {
      num -= steps * step;
      return *this;
    }

    bool operator==(const iterator &other) const noexcept {
      return positive ? (num >= other.num) : (num <= other.num);
    }

    T operator*() { return num; }

  private:
    T num;
    T step;
    bool positive;
  };

  using reverse_iterator = iterator;

  NumberRange(T stop)
    requires(std::is_integral_v<T>)
      : start{T(0)}, stop{stop}, step{stop > T(0) ? T(1) : T(-1)} {}
  NumberRange(T start, T stop)
    requires(std::is_integral_v<T>)
      : start{start}, stop{stop}, step{stop > T(0) ? T(1) : T(-1)} {}
  NumberRange(T start, T stop, T step) : start{start}, stop{stop}, step{step} {}

  iterator begin() const noexcept { return iterator{start, step}; }

  iterator end() const noexcept { return iterator{stop, step}; }

  reverse_iterator rbegin() const noexcept {
    return iterator{stop - step, -step};
  }

  reverse_iterator rend() const noexcept {
    return iterator{start - step, -step};
  }

private:
  T start;
  T stop;
  T step;
};
