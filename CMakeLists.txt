cmake_minimum_required(VERSION 3.5)
project(Console)

set(CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(SOURCE_FILES shell.c shellutil.c shellutil.h)
add_executable(Console ${SOURCE_FILES})