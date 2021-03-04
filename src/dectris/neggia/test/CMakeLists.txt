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


enable_testing()

execute_process(
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/h5-testfiles ${CMAKE_CURRENT_BINARY_DIR}/h5-testfiles
)

add_executable(Test_Dataset Test_Dataset.cpp DatasetsFixture.cpp)
target_link_libraries(Test_Dataset neggia_static z gtest gtest_main pthread)
add_test(Test_Dataset Test_Dataset)

add_executable(Test_H5DataspaceMsg Test_H5DataspaceMsg.cpp)
target_link_libraries(Test_H5DataspaceMsg neggia_static z gtest gtest_main pthread)
add_test(Test_H5DataspaceMsg Test_H5DataspaceMsg)

add_executable(Test_H5FilterMsg Test_H5FilterMsg.cpp)
target_link_libraries(Test_H5FilterMsg neggia_static z gtest gtest_main pthread)
add_test(Test_H5FilterMsg Test_H5FilterMsg)

add_executable(Test_H5ObjectHeader Test_H5ObjectHeader.cpp)
target_link_libraries(Test_H5ObjectHeader neggia_static z gtest gtest_main pthread)
add_test(Test_H5ObjectHeader Test_H5ObjectHeader)

add_executable(Test_XdsPlugin Test_XdsPlugin.cpp DatasetsFixture.cpp $<TARGET_OBJECTS:NEGGIA_COMPRESSION_ALGORITHMS>)
target_link_libraries(Test_XdsPlugin z dl gtest gtest_main pthread)
ADD_DEFINITIONS(-DPATH_TO_XDS_PLUGIN=\"${DECTRIS_NEGGIA_XDS_PLUGIN}\")
add_test(Test_XdsPlugin Test_XdsPlugin)
