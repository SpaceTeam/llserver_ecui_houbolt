cmake_minimum_required(VERSION 3.13)
project(TXV_ECUI_LLSERVER)

find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 17)

if(NOT TARGET spdlog)
    # Stand-alone build
    find_package(spdlog REQUIRED)
endif()

include_directories(include)

file(GLOB sources ./*.cpp src/*.cpp include/*.h)

add_executable(${PROJECT_NAME} ${sources})
#spdlog_enable_warnings(${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads spdlog::spdlog)