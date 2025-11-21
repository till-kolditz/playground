CMAKE_MINIMUM_REQUIRED(VERSION 4.0.0)

PROJECT(
  Exercises
  LANGUAGES CXX
)

INCLUDE(../cmake/base.cmake)

ADD_EXECUTABLE(hardware_concurrency hardware_concurrency.cc)
ADD_EXECUTABLE(hardware_interference_size hardware_interference_size.cc)
