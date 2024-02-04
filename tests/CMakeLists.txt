cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_LIST_DIR}/../cmake.modules" CACHE STRING "module-path")
project(test CXX)
enable_testing()

include(configure-target)

# get the files
include(files.cmake)

# find_package(GTest REQUIRED)
# include_directories(${GTEST_INCLUDE_DIRS}/include)

# Fetch gopgle test from repo
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY    https://github.com/google/googletest.git
  GIT_TAG           main
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
# We must adjust gtest variables, so cannot use FetchContent_MakeAvailable(googletest)
FetchContent_GetProperties(googletest)
if (NOT googletest_POPULATED)
    FetchContent_Populate(googletest)

    set(gtest_disable_pthreads ${BUILD_THREADS_DISABLED})
    add_definitions("-DGTEST_HAS_PTHREAD=0")
    set(INSTALL_GTEST OFF)

    add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
endif()


# Now simply link against gtest or gtest_main as needed.
add_executable(unittests ${SOURCES})
target_link_libraries(unittests gtest_main gmock_main ${PROJECT_LIBRARY})
include(GoogleTest)
gtest_discover_tests(unittests)
configure_target(unittests)
add_test(NAME test-1 COMMAND unittests "--gtest_output=xml:unittest.xml" WORKING_DIRECTORY "${CACHES_BUILD_PATH}")

print_config(unittests)
