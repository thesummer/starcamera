cmake_minimum_required(VERSION 2.8)
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

include_directories(include)
link_directories(/home/jan/Software/AptinaDevSuiteSDK/bin_linux /opt/python3.3/lib)
# link_directories(${CMAKE_SOURCE_DIR}/lib /opt/python3.3/lib)
# collect header files
FILE(GLOB starcamera_HEADER include/*.h)

add_subdirectory(src)



