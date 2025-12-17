#include <cmath>
#include <gtest/gtest.h>

#include "generator.hpp"

TEST(NumberRange, Zero) {
  for (auto num : NumberRange{0}) {
    ASSERT_TRUE(false) << "No iterations expected";
  }

  for (auto num : NumberRange{0, 0}) {
    ASSERT_TRUE(false) << "No iterations expected";
  }

  for (auto num : NumberRange{0, 0, 1}) {
    ASSERT_TRUE(false) << "No iterations expected";
  }

  for (auto num : NumberRange{1, 1, 1}) {
    ASSERT_TRUE(false) << "No iterations expected";
  }

  for (auto num : NumberRange{2, 1, 1}) {
    ASSERT_TRUE(false) << "No iterations expected";
  }

  for (auto num : NumberRange{0, 0, -1}) {
    ASSERT_TRUE(false) << "No iterations expected";
  }

  for (auto num : NumberRange{0, 0, -1}) {
    ASSERT_TRUE(false) << "No iterations expected";
  }

  for (auto num : NumberRange{-2, -1, -1}) {
    ASSERT_TRUE(false) << "No iterations expected";
  }
}

TEST(NumberRange, PositiveInt) {
  int expected = 0;
  for (auto num : NumberRange{10}) {
    EXPECT_EQ(expected, num);
    ++expected;
    ASSERT_LE(expected, 10);
  }
}

TEST(NumberRange, NegativeInt) {
  int expected = 0;
  for (auto num : NumberRange{-10}) {
    EXPECT_EQ(expected, num);
    --expected;
    ASSERT_GE(expected, -10);
  }
}

TEST(NumberRange, PositiveFloat) {
  float expected = 0;
  for (auto num : NumberRange{0.0f, 10.0f, 1.0f}) {
    EXPECT_EQ(expected, num);
    expected += 1.0f;
    ASSERT_LE(std::round(expected), 10.0f);
  }

  expected = 0;
  for (auto num : NumberRange{0.0f, 10.0f, .5f}) {
    EXPECT_EQ(expected, num);
    expected += .5f;
    ASSERT_LE(std::round(expected), 10.0f);
  }
}

TEST(NumberRange, NegativeFloat) {
  float expected = 0;
  for (auto num : NumberRange{0.0f, -10.0f, -1.0f}) {
    EXPECT_EQ(expected, num);
    expected -= 1.0f;
    ASSERT_GE(std::round(expected), -10.0f);
  }

  expected = 0;
  for (auto num : NumberRange{0.0f, -10.0f, -.1f}) {
    EXPECT_EQ(expected, num);
    expected -= .1f;
    ASSERT_GE(std::round(expected), -10.0f);
  }
}

TEST(NumberRange, PositiveDouble) {
  float expected = 0;
  for (auto num : NumberRange{0.0, 10.0, 1.0}) {
    EXPECT_EQ(expected, num);
    expected += 1.0;
    ASSERT_LE(std::round(expected), 10.0);
  }

  expected = 0;
  for (auto num : NumberRange{0.0, 10.0, .5}) {
    EXPECT_EQ(expected, num);
    expected += .5;
    ASSERT_LE(std::round(expected), 10.0);
  }
}

TEST(NumberRange, NegativeDouble) {
  float expected = 0;
  for (auto num : NumberRange{0.0f, -10.0f, -1.0f}) {
    EXPECT_EQ(expected, num);
    expected -= 1.0f;
    ASSERT_GE(std::round(expected), -10.0);
  }

  expected = 0;
  for (auto num : NumberRange{0.0f, -10.0f, -.1f}) {
    EXPECT_EQ(expected, num);
    expected -= .1f;
    ASSERT_GE(std::round(expected), -10.0);
  }
}
