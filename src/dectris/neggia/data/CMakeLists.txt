#  The MIT License (MIT)
#
#  Copyright (c) 2017 Dectris Ltd., Taefernweg 1, 5405 Baden-Daettwil, Switzerland.
#
#  Permission is hereby granted, free of charge, to any person obtaining a copy
#  of this software and associated documentation files (the "Software"), to deal
#  in the Software without restriction, including without limitation the rights
#  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#  copies of the Software, and to permit persons to whom the Software is
#  furnished to do so, subject to the following conditions:
#
#  The above copyright notice and this permission notice shall be included in all
#  copies or substantial portions of the Software.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
#  SOFTWARE.

set(SOURCES
        H5BTreeVersion2.cpp
        H5DataLayoutMsg.cpp
        H5DatatypeMsg.cpp
        H5DataspaceMsg.cpp
        H5FilterMsg.cpp
        H5FractalHeap.cpp
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

