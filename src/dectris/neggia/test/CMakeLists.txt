FIND_PACKAGE(HDF5 1.8.11 REQUIRED COMPONENTS C CXX HL)
include_directories(${HDF5_INCLUDE_DIRS})

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
find_library(GMOCK gmock) # libgmock.a is not found by FindGTest.cmake !!!
include_directories(${GMOCK_INCLUDE_DIRS})
set(GTEST_ALL ${GTEST_BOTH_LIBRARIES} ${GMOCK})
enable_testing()

set(SOURCES H5DatasetTestFixture.cpp bshuf_h5filter.c bshuf_h5filter.h Lz4Filter.hpp)

add_executable(Test_Dataset Test_Dataset.cpp ${SOURCES})
target_link_libraries(Test_Dataset neggia_static ${HDF5_LIBRARIES} z ${GTEST_ALL} pthread)
add_test(Test_Dataset Test_Dataset)

add_executable(Test_XdsPlugin Test_XdsPlugin.cpp ${SOURCES} $<TARGET_OBJECTS:NEGGIA_COMPRESSION_ALGORITHMS>)
target_link_libraries(Test_XdsPlugin ${HDF5_LIBRARIES} z ${GTEST_ALL} pthread)
ADD_DEFINITIONS(-DPATH_TO_XDS_PLUGIN=\"${DECTRIS_NEGGIA_XDS_PLUGIN}\")
add_test(Test_XdsPlugin Test_XdsPlugin)
