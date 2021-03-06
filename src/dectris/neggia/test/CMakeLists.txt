# SPDX-License-Identifier: MIT

execute_process(COMMAND
  "${CMAKE_COMMAND}" -E create_symlink
  "${CMAKE_CURRENT_SOURCE_DIR}/h5-testfiles"
  "${CMAKE_CURRENT_BINARY_DIR}/h5-testfiles"
  )
add_definitions(-DPATH_TO_XDS_PLUGIN=\"${DECTRIS_NEGGIA_XDS_PLUGIN}\")

add_executable(Test_Dataset Test_Dataset.cpp DatasetsFixture.cpp)
target_link_libraries(Test_Dataset
  gtest
  gtest_main
  neggia_static
  )
add_test(Test_Dataset Test_Dataset)

add_executable(Test_EigerData Test_EigerData.cpp)
target_link_libraries(Test_EigerData
  gtest
  gtest_main
  neggia_static
  )
add_test(Test_EigerData Test_EigerData)

add_executable(Test_H5DataspaceMsg Test_H5DataspaceMsg.cpp)
target_link_libraries(Test_H5DataspaceMsg
  gtest
  gtest_main
  neggia_static
  )
add_test(Test_H5DataspaceMsg Test_H5DataspaceMsg)

add_executable(Test_H5FilterMsg Test_H5FilterMsg.cpp)
target_link_libraries(Test_H5FilterMsg
  gtest
  gtest_main
  neggia_static
  )
add_test(Test_H5FilterMsg Test_H5FilterMsg)

add_executable(Test_H5ObjectHeader Test_H5ObjectHeader.cpp)
target_link_libraries(Test_H5ObjectHeader
  gtest
  gtest_main
  neggia_static
  )
add_test(Test_H5ObjectHeader Test_H5ObjectHeader)

add_executable(Test_XdsPlugin
  $<TARGET_OBJECTS:NEGGIA_COMPRESSION_ALGORITHMS>
  DatasetsFixture.cpp
  Test_XdsPlugin.cpp
  )
target_link_libraries(Test_XdsPlugin
  dl
  gtest
  gtest_main
  )
add_test(Test_XdsPlugin Test_XdsPlugin)
