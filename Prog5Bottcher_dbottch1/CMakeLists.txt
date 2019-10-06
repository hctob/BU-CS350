cmake_minimum_required(VERSION 2.8)
project(assn05)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_SOURCE_DIR ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

include_directories("${PROJECT_SOURCE_DIR}")

add_executable(lfs-simulator.bin
        main.cpp
        Drive.cpp Drive.h
        IMap.cpp IMap.h
        Segment.cpp Segment.h
        Block.cpp Block.h
        macros.cpp macros.h)
