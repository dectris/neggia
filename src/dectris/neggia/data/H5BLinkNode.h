// SPDX-License-Identifier: MIT

#ifndef H5BLINKNODE_H
#define H5BLINKNODE_H
#include <assert.h>
#include <string>
#include <vector>
#include "H5Object.h"

/// https://www.hdfgroup.org/HDF5/doc/H5.format.html#V1Btrees

class H5BLinkNode : public H5Object {
public:
    H5BLinkNode() = default;
    H5BLinkNode(const char* fileAddress, size_t offset);
    H5BLinkNode(const H5Object& other);
    int nodeType() const { return read_u8(4); }
    int nodeLevel() const { return read_u8(5); }
    int entriesUsed() const { return read_u16(6); }
    H5BLinkNode leftSilbling() const {
        return H5BLinkNode(fileAddress(), read_u64(8));
    }
    H5BLinkNode rightSilbling() const {
        return H5BLinkNode(fileAddress(), read_u64(16));
    }
    H5Object key(int i) const {
        // offset relative to local heap
        assert(nodeType() == 0);
        return H5Object(*this) + 24 + i * 16;
    }
    H5Object child(int i) const {
        // address of the child
        assert(nodeType() == 0);
        return H5Object(fileAddress(), read_u64(32 + i * 16));
    }
};

#endif  // H5BLINKNODE_H
