// SPDX-License-Identifier: MIT

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
    uint8_t version() const;
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
