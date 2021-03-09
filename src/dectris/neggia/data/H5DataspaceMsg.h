// SPDX-License-Identifier: MIT

#ifndef H5DATASPACEMSG_H
#define H5DATASPACEMSG_H
#include "H5Object.h"

/// https://www.hdfgroup.org/HDF5/doc/H5.format.html#DataspaceMessage

class H5DataspaceMsg : public H5Object {
public:
    H5DataspaceMsg() = default;
    H5DataspaceMsg(const H5Object&);
    H5DataspaceMsg(const char* fileAddress, size_t offset);
    uint8_t version() const;
    uint8_t rank() const;
    bool maxDims() const;
    uint64_t dim(int i) const;
    uint64_t maxDim(int i) const;
    constexpr static unsigned int TYPE_ID = 0x1;

private:
    void _init();
    uint64_t _dimsOffset;
};

#endif  // H5DATASPACEMSG_H
