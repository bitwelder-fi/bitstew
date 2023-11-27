cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
set( CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_LIST_DIR}/cmake.modules" )

# Set the project name here
project(meta)
set(PROJECT_LIBRARY meta_lib)

option(BUILD_WITH_TESTS "Build unit tests." ON)
option(BUILD_WITH_EXAMPLES "Build examples." OFF)
option(BUILD_MULTI_THREADED "Build multi-threaded library." ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(PROJECT_HEADER_PATH ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(PROJECT_SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(PROJECT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set(PROJECT_BUILD_PATH ${CMAKE_CURRENT_BINARY_DIR})
include(configure-platform)

add_subdirectory(src)

if (BUILD_WITH_EXAMPLES)
    add_subdirectory(examples)
endif()

if (BUILD_WITH_TESTS)
    add_subdirectory(tests)
endif()

