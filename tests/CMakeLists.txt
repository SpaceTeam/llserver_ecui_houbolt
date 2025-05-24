cmake_minimum_required(VERSION 3.13)

add_dependencies(${PROJECT_NAME} llserver_ecui_houbolt_lib)
# Enable testing
include(CTest)

# Fetch GoogleTest
include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.17.0 # Use latest stable release
)
FetchContent_MakeAvailable(googletest)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable strict warnings
add_compile_options(-Wall -Wextra -Wpedantic)

# Locate the test source files
file(GLOB_RECURSE TEST_SOURCES *.cpp *.h)

# Create test executable
add_executable(${PROJECT_NAME}_tests ${TEST_SOURCES})

# Link against the main project target and GoogleTest
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME}_tests PRIVATE
        ${PROJECT_NAME}_lib  # Link main project target
        GTest::gtest_main
        GTest::gmock
        GTest::gmock_main

        Threads::Threads
        -lstdc++fs
        -latomic
)


include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME}_tests)