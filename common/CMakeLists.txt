CMAKE_MINIMUM_REQUIRED(VERSION 4.0.0)

PROJECT(
  Common
  LANGUAGES CXX
)

INCLUDE(../cmake/base.cmake)

ADD_LIBRARY(common INTERFACE)
TARGET_INCLUDE_DIRECTORIES(common INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

ENABLE_TESTING()

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

ADD_EXECUTABLE(scopeguard_test scopeguard_test.cc)
TARGET_LINK_LIBRARIES(scopeguard_test common GTest::gtest_main)
ADD_TEST(NAME scopeguard_test COMMAND scopeguard_test)

INCLUDE(GoogleTest)
gtest_discover_tests(scopeguard_test)
