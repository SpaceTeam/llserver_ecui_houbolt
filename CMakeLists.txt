cmake_minimum_required(VERSION 3.13)
project(TXV_ECUI_LLSERVER)

find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

file(GLOB sources ./*.cpp src/*.cpp include/*.h)

add_executable(${PROJECT_NAME} ${sources})
target_link_libraries(${PROJECT_NAME} Threads::Threads)
