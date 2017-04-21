#  The MIT License (MIT)
#
#  Copyright (c) 2017 Dectris Ltd., Taefernweg 1, 5405 Baden-Daettwil, Switzerland.
#
#  Permission is hereby granted, free of charge, to any person obtaining a copy
#  of this software and associated documentation files (the "Software"), to deal
#  in the Software without restriction, including without limitation the rights
#  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#  copies of the Software, and to permit persons to whom the Software is
#  furnished to do so, subject to the following conditions:
#
#  The above copyright notice and this permission notice shall be included in all
#  copies or substantial portions of the Software.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
#  SOFTWARE.

cmake_minimum_required(VERSION 3.6)
project(neggia)

cmake_policy(SET CMP0005 NEW)
cmake_policy(SET CMP0003 NEW)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -O3")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -g -O3")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

enable_testing()
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(VERSION 1.0.1)
add_definitions(-DVERSION=\"${VERSION}\")

add_subdirectory(src)




