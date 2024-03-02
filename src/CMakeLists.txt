cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_LIST_DIR}/../cmake.modules" CACHE STRING "module-path")
include(configure-target)

project(${PROJECT_LIBRARY} VERSION 1.0.0 LANGUAGES CXX)

# get the files
include(files.cmake)

# BUILD_SHARED_LIBS drives the library type for add_Library
add_library(${PROJECT_LIBRARY} "")
target_sources(${PROJECT_LIBRARY}
    PRIVATE ${SOURCES} ${PRIVATE_HEADERS}
    PUBLIC ${HEADERS})
target_include_directories(${PROJECT_LIBRARY}
    PUBLIC ${PROJECT_HEADER_PATH}
    PRIVATE ${PROJECT_SOURCE_PATH}/include)

configure_target(${PROJECT_LIBRARY})
