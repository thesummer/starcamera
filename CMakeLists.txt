cmake_minimum_required(VERSION 2.8.1)
project(starcamera)

message("CMAKE_CXX_FLAGS_DEBUG is ${CMAKE_CXX_FLAGS_DEBUG}")
message("CMAKE_CXX_FLAGS_RELEASE is ${CMAKE_CXX_FLAGS_RELEASE}")
message("CMAKE_BUILD_TYPE is ${CMAKE_BUILD_TYPE}")

set(CMAKE_CXX_FLAGS "-Wall -std=c++0x")

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    set(BUILD_POSTFIX "_d")
else (CMAKE_BUILD_TYPE STREQUAL Debug)
    set(BUILD_POSTFIX "")
endif (CMAKE_BUILD_TYPE STREQUAL Debug)

if(PLATFORM STREQUAL Beagle)
    link_directories(lib/beaglebone)
else(PLATFORM STREQUAL Beagle)
    link_directories(lib /opt/python3.3/lib)
endif(PLATFORM STREQUAL Beagle)

include_directories(include)

# collect header files
FILE(GLOB starcamera_HEADER include/*.h)

add_subdirectory(src)



