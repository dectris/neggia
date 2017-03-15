set(SOURCES
        H5BTreeVersion2.cpp
        H5DataLayoutMsg.cpp
        H5DatatypeMsg.cpp
        H5DataspaceMsg.cpp
        H5FilterMsg.cpp
        H5FractalHeap.cpp
        H5HeaderMsg.cpp
        H5LinkInfoMessage.cpp
        H5LinkMsg.cpp
        H5Object.cpp
        H5ObjectHeader.cpp
        H5Path.cpp
        H5Superblock.cpp
        H5BLinkNode.cpp
        H5LocalHeap.cpp
        H5SymbolTableEntry.cpp
        H5SymbolTableNode.cpp
        JenkinsLookup3Checksum.cpp
        Decode.cpp
)

add_library(NEGGIA_DATA OBJECT ${SOURCES})

#add_subdirectory(user)
#add_subdirectory(plugin)
#
#add_library(simpleh5 STATIC ${SOURCES} $<TARGET_OBJECTS:HL>)
#add_library(simpleh5_shared SHARED ${SOURCES} $<TARGET_OBJECTS:HL>)
#set_target_properties(simpleh5_shared PROPERTIES OUTPUT_NAME simpleh5)
#add_library(simpleh5_object OBJECT  ${SOURCES})
#
#IF ( CMAKE_ENABLE_TESTING )
#  enable_testing()
#  add_subdirectory(test)
#  add_subdirectory(playground)
#ENDIF()
#
