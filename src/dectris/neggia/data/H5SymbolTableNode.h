// SPDX-License-Identifier: MIT

#ifndef H5SYMBOLTABLENODE_H
#define H5SYMBOLTABLENODE_H
#include "H5Object.h"
#include "H5SymbolTableEntry.h"

/// https://www.hdfgroup.org/HDF5/doc/H5.format.html#SymbolTable

class H5SymbolTableNode : public H5Object {
public:
    H5SymbolTableNode() = default;
    H5SymbolTableNode(const char* fileAddress, size_t offset);
    H5SymbolTableNode(const H5Object& other);
    int numberOfSymbols() const;
    H5SymbolTableEntry entry(int i) const;
};

#endif  // H5SYMBOLTABLENODE_H
