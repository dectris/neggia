ADD_DEFINITIONS(-DTEST_PLUGIN_LIBRARY=\"${CMAKE_CURRENT_BINARY_DIR}/../libdectrish5toxds.so\")


add_executable(plugin_test plugin_test.cpp)
target_link_libraries(plugin_test dl simpleh5)
