CMAKE_MINIMUM_REQUIRED(VERSION 4.0.0)

PROJECT(
  Exercises
  LANGUAGES CXX
)

INCLUDE(../cmake/base.cmake)

ADD_EXECUTABLE(hardware_concurrency hardware_concurrency.cc)
ADD_EXECUTABLE(hardware_interference_size hardware_interference_size.cc)
ADD_EXECUTABLE(stream_redirect stream_redirect.cc)
ADD_EXECUTABLE(lru_cache lru_cache.cc)
ADD_EXECUTABLE(word_frequencies word_frequencies.cc)

ADD_EXECUTABLE(lru_cache_test lru_cache_test.cc)
TARGET_LINK_LIBRARIES(lru_cache_test GTest::gtest_main)

ADD_EXECUTABLE(lru_cache_bench lru_cache_bench.cc)
TARGET_LINK_LIBRARIES(lru_cache_bench benchmark)

INCLUDE(FetchContent)
FetchContent_Declare(
  simdjson
  GIT_REPOSITORY https://github.com/simdjson/simdjson.git
  GIT_TAG tags/v4.2.2
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(simdjson)

ADD_EXECUTABLE(simdjson_print simdjson_print.cc)
TARGET_LINK_LIBRARIES(simdjson_print simdjson)
