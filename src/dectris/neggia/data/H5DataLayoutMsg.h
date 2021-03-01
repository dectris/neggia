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

#ifndef H5DATALAYOUTMSG_H
#define H5DATALAYOUTMSG_H
#include "H5BLinkNode.h"
#include "H5Object.h"

/// https://www.hdfgroup.org/HDF5/doc/H5.format.html#LayoutMessage

class H5DataLayoutMsg : public H5Object {
public:
    H5DataLayoutMsg() = default;
    H5DataLayoutMsg(const char* fileAddress, size_t offset);
    H5DataLayoutMsg(const H5Object&);
    uint8_t layoutClass() const;

    /// for raw and contigous data  (layout class 0,1)
    size_t dataSize() const;
    const char* dataAddress() const;

    /// for chunked data (layout class 2)
    uint8_t chunkDims() const;
    H5BLinkNode chunkBTree() const;
    uint32_t chunkDim(int i) const;
    constexpr static unsigned int TYPE_ID = 0x8;

private:
    void _init();
};

#endif  // H5DATALAYOUTMSG_H
