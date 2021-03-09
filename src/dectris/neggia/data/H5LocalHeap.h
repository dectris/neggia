// SPDX-License-Identifier: MIT

#ifndef H5LOCALHEAP_H
#define H5LOCALHEAP_H
#include "H5Object.h"

/// https://www.hdfgroup.org/HDF5/doc/H5.format.html#LocalHeap

class H5LocalHeap : public H5Object {
public:
    H5LocalHeap() = default;
    H5LocalHeap(const char* fileAddress, size_t offset);
    H5LocalHeap(const H5Object& other);
    const char* data(size_t offset) const { return _dataSegment + offset; }

private:
    const char* _dataSegment;
};

#endif  // H5LOCALHEAP_H
