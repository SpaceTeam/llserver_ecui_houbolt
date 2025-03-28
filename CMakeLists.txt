cmake_minimum_required(VERSION 3.13)
project(llserver_ecui_houbolt)

find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED)
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall")

include_directories(include include/can_houbolt)

file(GLOB_RECURSE sources main.cpp src/*.cpp src/*.c include/*.h)

if(NO_PYTHON)
  MESSAGE(STATUS "Python features are not used")
  add_compile_definitions(NO_PYTHON)
  list(REMOVE_ITEM sources ${CMAKE_CURRENT_SOURCE_DIR}/src/drivers/PythonController.cpp)
else()
  include_directories(/usr/include/python3.10)
endif()

add_executable(${PROJECT_NAME} ${sources})

option(TEST "test with simple CAN Driver simulation" OFF)
if(TEST)
   add_compile_definitions(TEST_LLSERVER)
endif()

if(NO_CANLIB)
  MESSAGE(STATUS "Kvaser CanLib is not used")
  add_compile_definitions(NO_CANLIB)
endif()


if(UNIX AND NOT APPLE)
   set(LINUX TRUE)
endif()

if(LINUX)
   target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads -lstdc++fs -latomic)
      
   if(NOT NO_CANLIB)
      target_link_libraries(${PROJECT_NAME} PRIVATE -lcanlib)
   endif()
   if(NOT NO_PYTHON)
      target_link_libraries(${PROJECT_NAME} PRIVATE -lpython3.10)
   endif()
else()
   target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
endif()
