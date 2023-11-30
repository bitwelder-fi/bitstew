cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_LIST_DIR}/../cmake.modules" CACHE STRING "module-path")
project(test CXX)
enable_testing()

include(configure-target)

set (SOURCES
    utils/trace_printer_mock.hpp
    test_utils.hpp
    test_main.cpp
    test_log_fixtures.hpp
    test_log.cpp
    test_thread_pool.cpp
)

# Fetch gopgle test from repo
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY    https://github.com/google/googletest.git
  GIT_TAG           main
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


# Now simply link against gtest or gtest_main as needed.
add_executable(unittests ${SOURCES})
target_link_libraries(unittests GTest::gtest_main GTest::gmock_main ${PROJECT_LIBRARY})
configure_target(unittests)
add_test(NAME test-1 COMMAND unittests "--gtest_output=xml:unittest.xml" WORKING_DIRECTORY "${CACHES_BUILD_PATH}")

# add_executable(unittests ${SOURCES})
# target_link_libraries(unittests GTest::gtest_main GTest::gmock_main ${PROJECT_LIBRARY})
# include(GoogleTest)
# gtest_discover_tests(unittests)
