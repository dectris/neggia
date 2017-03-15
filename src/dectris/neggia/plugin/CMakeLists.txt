set (SOURCES
    H5Error.h
    H5ToXds.h
    H5ToXds.cpp
)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_library(dectris-neggia SHARED ${SOURCES})
target_link_libraries(dectris-neggia neggia_static)
set_target_properties(dectris-neggia PROPERTIES PREFIX "" SUFFIX ".so")
install(TARGETS dectris-neggia LIBRARY DESTINATION lib)
set(DECTRIS_NEGGIA_XDS_PLUGIN ${CMAKE_CURRENT_BINARY_DIR}/dectris-neggia.so CACHE INTERNAL "")
