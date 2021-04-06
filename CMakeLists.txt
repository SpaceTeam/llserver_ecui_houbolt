cmake_minimum_required(VERSION 3.13)
project(llserver_ecui_houbolt)

find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall")

include_directories(include include/can_houbolt)

file(GLOB_RECURSE sources ./*.cpp src/*.cpp include/*.h)


add_executable(${PROJECT_NAME} ${sources})


if(UNIX AND NOT APPLE)
   set(LINUX TRUE)
endif()

if(LINUX)
   target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads -lstdc++fs)
else()
   target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
endif()


##----WITH BOOST
# cmake_minimum_required(VERSION 3.13)
# project(llserver_ecui_houbolt)

# find_package(Threads REQUIRED)

# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall")

# #if(NOT TARGET spdlog)
# #    # Stand-alone build
# #    find_package(spdlog REQUIRED)
# #endif()

# include_directories(include)

# file(GLOB sources ./*.cpp src/*.cpp include/*.h)

# set(Boost_USE_STATIC_LIBS OFF)
# set(Boost_USE_MULTITHREADED ON)
# set(Boost_USE_STATIC_RUNTIME OFF)
# find_package(Boost 1.45.0 COMPONENTS system)

# if(Boost_FOUND)
#     add_executable(${PROJECT_NAME} ${sources})
#     #spdlog_enable_warnings(${PROJECT_NAME})


#     if(UNIX AND NOT APPLE)
#         set(LINUX TRUE)
#     endif()

#     if(LINUX)
#         target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads -lstdc++fs Boost::system)#spdlog::spdlog)
#     else()
#         target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads Boost::system)#spdlog::spdlog)
#     endif()
# endif()