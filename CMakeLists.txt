cmake_minimum_required(VERSION 3.13)
project(llserver_ecui_houbolt)

find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED)
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall")

include_directories(include include/can_houbolt)

file(GLOB_RECURSE sources src/*.cpp src/*.c include/*.h)


if(NO_PYTHON)
  MESSAGE(STATUS "Python features are not used")
  add_compile_definitions(NO_PYTHON)
  list(REMOVE_ITEM sources ${CMAKE_CURRENT_SOURCE_DIR}/src/drivers/PythonController.cpp)
else()
  find_package(Python3 COMPONENTS Development REQUIRED)
  include_directories(${Python3_INCLUDE_DIRS})
endif()

# Define a library for the main project
add_library(${PROJECT_NAME}_lib STATIC ${sources})

include(FetchContent)

FetchContent_Declare(
        readerwriterqueue
        GIT_REPOSITORY    https://github.com/cameron314/readerwriterqueue
        GIT_TAG           v1.0.7
)

FetchContent_MakeAvailable(readerwriterqueue)

target_link_libraries(${PROJECT_NAME}_lib PUBLIC readerwriterqueue)

# Create an executable that links to this library
add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_lib)
option(E2E_TEST  "test with simple CAN Driver simulation" OFF)
option(NO_INFLUX "Disable influxdb logging" OFF)
option(NO_CANLIB "Disable Kvaser CANLIB" OFF)
option(NO_PYTHON "Disable Python" OFF)
if(E2E_TEST)
   add_compile_definitions(TEST_LLSERVER)
endif()

if(NO_CANLIB)
  MESSAGE(STATUS "Kvaser CanLib is not used")
  add_compile_definitions(NO_CANLIB)
endif()

if(NO_INFLUX)
    MESSAGE(STATUS "InfluxDB is not used")
    add_compile_definitions(NO_INFLUX)
endif()

if(UNIX AND NOT APPLE)
   set(LINUX TRUE)
endif()

if(LINUX)
   target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads -latomic)
      
   if(NOT NO_CANLIB)
      target_link_libraries(${PROJECT_NAME} PRIVATE -lcanlib)
   endif()
   if(NOT NO_PYTHON)
       target_link_libraries(${PROJECT_NAME}  PRIVATE ${Python3_LIBRARIES})
   endif()
else()
   target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
endif()

if(TEST)
    enable_testing()
    add_subdirectory(tests)
endif ()