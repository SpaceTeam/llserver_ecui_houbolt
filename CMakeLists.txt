cmake_minimum_required(VERSION 3.13)
project(TXV_ECUI_LLSERVER)

find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall")

#if(NOT TARGET spdlog)
#    # Stand-alone build
#    find_package(spdlog REQUIRED)
#endif()

include_directories(include)

file(GLOB sources ./*.cpp src/*.cpp include/*.h)

add_executable(${PROJECT_NAME} ${sources})
#spdlog_enable_warnings(${PROJECT_NAME})


if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

if(LINUX)
    target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads wiringPi)#spdlog::spdlog)
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)#spdlog::spdlog)
endif()

