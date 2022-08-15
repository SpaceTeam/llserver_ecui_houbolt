cmake_minimum_required(VERSION 3.16)
project(llserver_ecui_houbolt)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED)
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -pedantic")

include_directories(include)

file(GLOB_RECURSE SRC_FILES src/*.cpp)
file(GLOB_RECURSE INCL_FILES include/*.hpp include/*.h)
#file(GLOB LIB_FILES lib/[a-zA-Z]*)
file(GLOB_RECURSE TEST_FILES test/*)

#add_subdirectory(${LIB_FILES})

find_package(Threads REQUIRED)

add_executable(${PROJECT_NAME} ${SRC_FILES} ${INCL_FILES})

add_library(${PROJECT_NAME}_lib ${SRC_FILES} ${INCL_FILES})
add_executable(test_${PROJECT_NAME} ${TEST_FILES})

# Google Test stuff
include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_test(Name test_${PROJECT_NAME} COMMAND test_${PROJECT_NAME})

include(GoogleTest)
gtest_discover_tests(test_${PROJECT_NAME})

# Libary linking

if(UNIX AND NOT APPLE)
    if(NO_CANLIB)
        #target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads -lstdc++fs -latomic -lpython3.8)
        target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads -lstdc++fs -latomic)
    else()
        #target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads -lstdc++fs -latomic -lcanlib -lpython3.8)
        target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads -lstdc++fs -latomic)
    endif()
else()
   target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
endif()

target_link_libraries(test_${PROJECT_NAME} PUBLIC ${PROJECT_NAME}_lib GTest::gtest_main -latomic -lpthread -lstdc++fs)


