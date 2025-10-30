# Basic CMake settings for all projects

SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

FIND_PROGRAM(MOLD_PATH mold)
IF(EXISTS ${MOLD_PATH})
    MESSAGE(STATUS "mold found at: ${MOLD_PATH}")
    SET(CMAKE_LINKER_TYPE MOLD)
ELSE()
    MESSAGE(STATUS "mold not found")
ENDIF()

FIND_PROGRAM(CMAKE_PATH ccache)
IF(EXISTS ${CMAKE_PATH})
    MESSAGE(STATUS "ccache found at: ${CMAKE_PATH}")
    SET(CMAKE_CXX_COMPILER_LAUNCHER "ccache")
    SET(CMAKE_C_COMPILER_LAUNCHER "ccache")
ELSE()
    MESSAGE(STATUS "ccache not found")
ENDIF()

SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

INCLUDE(FetchContent)
FetchContent_Declare(
  googletest
  #URL https://github.com/google/googletest/releases/download/v1.17.0/googletest-1.17.0.tar.gz
  URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.tar.gz
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
SET(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(
  benchmark
  URL https://github.com/google/benchmark/archive/refs/tags/v1.9.4.tar.gz
)
FetchContent_MakeAvailable(benchmark)
