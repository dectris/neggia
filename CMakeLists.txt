cmake_minimum_required(VERSION 3.6)
project(neggia)

cmake_policy(SET CMP0005 NEW)
cmake_policy(SET CMP0003 NEW)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -O3")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -g -O3")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

enable_testing()
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_subdirectory(src)




