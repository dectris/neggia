// SPDX-License-Identifier: MIT

#ifndef H5DATATYPEMSG_H
#define H5DATATYPEMSG_H
#include "H5Object.h"
/// https://www.hdfgroup.org/HDF5/doc/H5.format.html#DatatypeMessage

class H5DatatypeMsg : public H5Object {
public:
    H5DatatypeMsg() = default;
    H5DatatypeMsg(const char* fileAddress, size_t offset);
    H5DatatypeMsg(const H5Object&);
    unsigned int version() const;
    unsigned int typeId() const;
    unsigned int dataSize() const;
    bool isSigned() const;
    constexpr static unsigned int TYPE_ID = 0x3;

private:
    void _init();
};

#endif  // H5DATATYPEMSG_H
