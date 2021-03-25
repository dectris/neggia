# SPDX-License-Identifier: MIT

add_library(NEGGIA_DATA OBJECT
  Decode.cpp
  H5BLinkNode.cpp
  H5BTreeVersion2.cpp
  H5DataLayoutMsg.cpp
  H5DataspaceMsg.cpp
  H5DatatypeMsg.cpp
  H5ExtensibleArray.cpp
  H5FilterMsg.cpp
  H5FixedArray.cpp
  H5FractalHeap.cpp
  H5LinkInfoMessage.cpp
  H5LinkMsg.cpp
  H5LocalHeap.cpp
  H5Object.cpp
  H5ObjectHeader.cpp
  H5Path.cpp
  H5Superblock.cpp
  H5SymbolTableEntry.cpp
  H5SymbolTableNode.cpp
  JenkinsLookup3Checksum.cpp
  PathResolverV0.cpp
  PathResolverV2.cpp
  )
