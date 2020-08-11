# cmake_minimum_required(VERSION 3.15)
project(walksatSNC)
project(bench_walksat)

set(CMAKE_CXX_STANDARD 17)

if (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif()

add_definitions(-DRAND_DEVICE)

add_executable(walksatSNC main.cpp walksat.cpp walksat.h walksatSNC.cpp walksatSNC.h)
add_executable(bench_walksat bench_walksat.cpp walksat.cpp walksat.h walksatSNC.cpp walksatSNC.h)
