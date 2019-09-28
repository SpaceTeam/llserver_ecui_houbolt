cmake_minimum_required(VERSION 3.14)
project(TXV_ECUI_LLSERVER)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

file(GLOB sources ./*.cpp src/*.cpp include/*.h)

add_executable(${PROJECT_NAME} ${sources})