// SPDX-License-Identifier: MIT

#ifndef H5SYMBOLTABLEENTRY_H
#define H5SYMBOLTABLEENTRY_H
#include <string>
#include <vector>
#include "H5ObjectHeader.h"

/// See https://www.hdfgroup.org/HDF5/doc/H5.format.html#SymbolTableEntry

class H5SymbolTableEntry : public H5Object {
public:
    enum CACHE_TYPE { DATA = 0, GROUP = 1, LINK = 2 };

    H5SymbolTableEntry() = default;
    H5SymbolTableEntry(const char* fileAddress, size_t offset);
    H5SymbolTableEntry(const H5Object& other);
    size_t linkNameOffset() const;
    H5ObjectHeader objectHeader() const;
    CACHE_TYPE cacheType() const;
    H5Object scratchSpace() const;
    uint64_t getAddressOfBTree() const;
    uint64_t getAddressOfHeap() const;
    uint32_t getOffsetToLinkValue() const;
    H5SymbolTableEntry find(const std::string& entry) const;
};

#endif  // H5SYMBOLTABLEENTRY_H
