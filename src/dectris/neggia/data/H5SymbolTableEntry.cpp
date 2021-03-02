/**
MIT License

Copyright (c) 2017 DECTRIS Ltd.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "H5SymbolTableEntry.h"
#include <string.h>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include "H5BLinkNode.h"
#include "H5DataLayoutMsg.h"
#include "H5LocalHeap.h"
#include "H5Superblock.h"
#include "H5SymbolTableNode.h"
#include "assert.h"

H5SymbolTableEntry::H5SymbolTableEntry(const char* fileAddress, size_t offset)
      : H5Object(fileAddress, offset) {}

H5SymbolTableEntry::H5SymbolTableEntry(const H5Object& other)
      : H5Object(other) {}

size_t H5SymbolTableEntry::linkNameOffset() const {
    return read_u64(0);
}

H5ObjectHeader H5SymbolTableEntry::objectHeader() const {
    size_t offset = read_u64(8);
    return H5ObjectHeader(fileAddress(), offset);
}

H5SymbolTableEntry::CACHE_TYPE H5SymbolTableEntry::cacheType() const {
    return (CACHE_TYPE)(read_u16(16));
}

H5Object H5SymbolTableEntry::scratchSpace() const {
    return at(24);
}

uint64_t H5SymbolTableEntry::getAddressOfBTree() const {
    assert(cacheType() == GROUP);
    return scratchSpace().read_u64(0);
}

uint64_t H5SymbolTableEntry::getAddressOfHeap() const {
    assert(cacheType() == GROUP);
    return scratchSpace().read_u64(8);
}

uint32_t H5SymbolTableEntry::getOffsetToLinkValue() const {
    assert(cacheType() == LINK);
    return scratchSpace().read_u32(0);
}

H5SymbolTableEntry H5SymbolTableEntry::find(const std::string& entry) const {
    assert(cacheType() == 1);  // makes sense only for groups

    H5BLinkNode bTree(fileAddress(), scratchSpace().read_i64(0));
    assert(bTree.nodeType() == 0);
    H5LocalHeap treeHeap(fileAddress(), scratchSpace().read_i64(8));
    while (bTree.nodeLevel() > 0) {
        bool found = false;
        for (int i = 1; i <= bTree.entriesUsed(); ++i) {
            size_t off = bTree.key(i).read_u64(0);
            std::string key = std::string(treeHeap.data(off));
            if (entry <= key) {
                bTree = bTree.child(i - 1);
                found = true;
                break;
            }
        }
        if (!found)
            throw std::runtime_error("Not found");
    }

    {
        int i;
        for (i = 1; i <= bTree.entriesUsed(); ++i) {
            size_t off = bTree.key(i).read_u64(0);
            std::string key = std::string(treeHeap.data(off));
            if (entry <= key) {
                break;
            }
        }
        if (i > bTree.entriesUsed())
            throw std::out_of_range("Not found");
        H5SymbolTableNode symbolTableNode(bTree.child(i - 1));
        for (i = 0; i < symbolTableNode.numberOfSymbols(); ++i) {
            H5SymbolTableEntry retVal(symbolTableNode.entry(i));
            size_t off = retVal.linkNameOffset();
            std::string key = std::string(treeHeap.data(off));
            if (key == entry) {
                return retVal;
            }
        }
        throw std::out_of_range("Not found");
    }
}
