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

find_package(DectrisHdf5 REQUIRED)
include_directories(${DECTRIS_HDF5_INCLUDE_DIRS})

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
find_library(GMOCK gmock) # libgmock.a is not found by FindGTest.cmake !!!
include_directories(${GMOCK_INCLUDE_DIRS})
set(GTEST_ALL ${GTEST_BOTH_LIBRARIES} ${GMOCK})
enable_testing()

set(SOURCES H5DatasetTestFixture.cpp bshuf_h5filter.c bshuf_h5filter.h Lz4Filter.hpp)

add_executable(Test_Dataset Test_Dataset.cpp ${SOURCES})
target_link_libraries(Test_Dataset neggia_static ${DECTRIS_HDF5_LIBRARIES} z ${GTEST_ALL} pthread)
add_test(Test_Dataset Test_Dataset)

add_executable(Test_XdsPlugin Test_XdsPlugin.cpp ${SOURCES} $<TARGET_OBJECTS:NEGGIA_COMPRESSION_ALGORITHMS>)
target_link_libraries(Test_XdsPlugin ${DECTRIS_HDF5_LIBRARIES} z dl ${GTEST_ALL} pthread)
ADD_DEFINITIONS(-DPATH_TO_XDS_PLUGIN=\"${DECTRIS_NEGGIA_XDS_PLUGIN}\")
add_test(Test_XdsPlugin Test_XdsPlugin)
