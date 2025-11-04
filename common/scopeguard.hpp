#pragma once

#include <utility>

namespace common {

template <typename Func> class ScopeGuard {
  Func m_cleanup;
  bool m_active{true};

public:
  explicit ScopeGuard(Func &&cleanup) : m_cleanup{std::move(cleanup)} {}

  ScopeGuard(ScopeGuard const &) = delete;
  ScopeGuard &operator=(ScopeGuard const &) = delete;

  ScopeGuard(ScopeGuard &&other) = delete;
  ScopeGuard &operator=(ScopeGuard &&other) = delete;

  ~ScopeGuard() { reset(); }

  void release() { m_active = false; }

  void reset() {
    if (m_active) {
      release();
      m_cleanup();
    }
  }
};

template <typename Func>
inline ScopeGuard<Func> MakeScopeGuard(Func &&cleanup) {
  return ScopeGuard<Func>{std::forward<Func>(cleanup)};
}

} // namespace common
