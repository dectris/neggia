# SPDX-License-Identifier: MIT

add_definitions(-DVERSION=\"${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}\")
add_library(dectris-neggia MODULE
  H5Error.h
  H5ToXds.cpp
  H5ToXds.h
  )
target_link_libraries(dectris-neggia neggia_static)
set_target_properties(dectris-neggia PROPERTIES PREFIX "" SUFFIX ".so")
install(TARGETS dectris-neggia LIBRARY DESTINATION lib)
set(DECTRIS_NEGGIA_XDS_PLUGIN ${CMAKE_CURRENT_BINARY_DIR}/dectris-neggia.so CACHE INTERNAL "")
