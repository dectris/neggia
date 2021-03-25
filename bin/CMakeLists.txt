# SPDX-License-Identifier: MIT

add_executable(check_h5_plugin
  $<TARGET_OBJECTS:NEGGIA_COMPRESSION_ALGORITHMS>
  $<TARGET_OBJECTS:NEGGIA_DATA>
  $<TARGET_OBJECTS:NEGGIA_PLUGIN>
  $<TARGET_OBJECTS:NEGGIA_USER>
  check_h5_plugin.cpp
  )

target_link_libraries(check_h5_plugin
  dl
)