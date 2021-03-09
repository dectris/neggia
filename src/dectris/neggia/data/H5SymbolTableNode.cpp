// SPDX-License-Identifier: MIT

#include "H5SymbolTableNode.h"
#include <assert.h>

H5SymbolTableNode::H5SymbolTableNode(const char* fileAddress, size_t offset)
      : H5Object(fileAddress, offset) {
    assert(std::string(address(), 4) == "SNOD");
}

H5SymbolTableNode::H5SymbolTableNode(const H5Object& other) : H5Object(other) {
    assert(std::string(address(), 4) == "SNOD");
}

int H5SymbolTableNode::numberOfSymbols() const {
    return read_u16(6);
}

H5SymbolTableEntry H5SymbolTableNode::entry(int i) const {
    return H5SymbolTableEntry(at(8 + i * 40));
}
