CMAKE_MINIMUM_REQUIRED(VERSION 4.0.0)

PROJECT(
  Exercises
  LANGUAGES CXX
)

INCLUDE(../cmake/base.cmake)

ADD_EXECUTABLE(hardware_concurrency hardware_concurrency.cc)
ADD_EXECUTABLE(hardware_interference_size hardware_interference_size.cc)
ADD_EXECUTABLE(stream_redirect stream_redirect.cc)

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