CMAKE_MINIMUM_REQUIRED(VERSION 3.28)

PROJECT(ObjectStore)

SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

INCLUDE(../base.cmake)

IF(NOT TARGET common)
    ADD_SUBDIRECTORY(../common ${CMAKE_CURRENT_BINARY_DIR}/common)
ENDIF()

ENABLE_TESTING()

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

ADD_EXECUTABLE(objectstore_test
    objectstore_test.cc
    objectstore.cc
    )
TARGET_LINK_LIBRARIES(objectstore_test common GTest::gtest_main)
ADD_TEST(NAME objectstore_test COMMAND objectstore_test)

INCLUDE(GoogleTest)
gtest_discover_tests(objectstore_test)

#ADD_EXECUTABLE(objectstore_bench objectstore_bench.cc)
#TARGET_LINK_LIBRARIES(objectstore_bench benchmark::benchmark)
