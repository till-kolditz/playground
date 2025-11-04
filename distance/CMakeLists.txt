CMAKE_MINIMUM_REQUIRED(VERSION 3.28)

PROJECT(Distance)

INCLUDE(../base.cmake)

ENABLE_TESTING()

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

ADD_EXECUTABLE(distance_test distance_test.cc)
TARGET_LINK_LIBRARIES(distance_test GTest::gtest_main)

INCLUDE(GoogleTest)
gtest_discover_tests(distance_test)

ADD_EXECUTABLE(distance_bench distance_bench.cc)
TARGET_LINK_LIBRARIES(distance_bench benchmark::benchmark)
