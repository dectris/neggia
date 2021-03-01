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
