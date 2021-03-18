# SPDX-License-Identifier: MIT

add_subdirectory(compression_algorithms)
add_subdirectory(data)
add_subdirectory(plugin)
add_subdirectory(user)

add_library(neggia_static STATIC
  $<TARGET_OBJECTS:NEGGIA_COMPRESSION_ALGORITHMS>
  $<TARGET_OBJECTS:NEGGIA_DATA>
  $<TARGET_OBJECTS:NEGGIA_USER>
  )

if(BUILD_TESTING)
  add_subdirectory(test)
endif()
