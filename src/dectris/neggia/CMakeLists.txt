add_subdirectory(compression_algorithms)
add_subdirectory(data)
add_subdirectory(user)
add_subdirectory(plugin)
add_subdirectory(test)

add_library(neggia_static $<TARGET_OBJECTS:NEGGIA_DATA> $<TARGET_OBJECTS:NEGGIA_USER> $<TARGET_OBJECTS:NEGGIA_COMPRESSION_ALGORITHMS>)
