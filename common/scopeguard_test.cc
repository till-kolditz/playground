#include <gtest/gtest.h>

#include <scopeguard.hpp>

TEST(ScopeGuard, Default) {
  int x = 0;
  {
    auto guard = common::MakeScopeGuard([&x] { x = 42; });
    EXPECT_EQ(x, 0);
  }
  EXPECT_EQ(x, 42);
}

TEST(ScopeGuard, Reset) {
  int x = 0;
  {
    auto guard = common::MakeScopeGuard([&x] { x = 42; });
    EXPECT_EQ(x, 0);
    guard.reset();
    EXPECT_EQ(x, 42);
  }
  EXPECT_EQ(x, 42);
}

TEST(ScopeGuard, Release) {
  int x = 0;
  {
    auto guard = common::MakeScopeGuard([&x] { x = 42; });
    EXPECT_EQ(x, 0);
    guard.release();
    EXPECT_EQ(x, 0);
  }
  EXPECT_EQ(x, 0);
}
