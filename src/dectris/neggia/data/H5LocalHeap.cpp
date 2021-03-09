// SPDX-License-Identifier: MIT

#include "H5LocalHeap.h"
#include <assert.h>

H5LocalHeap::H5LocalHeap(const char* fileAddress, size_t offset)
      : H5Object(fileAddress, offset) {
    assert(std::string(address(), 4) == "HEAP");
    _dataSegment = this->fileAddress() + read_u64(24);
}

H5LocalHeap::H5LocalHeap(const H5Object& other) : H5Object(other) {
    assert(std::string(address(), 4) == "HEAP");
    _dataSegment = this->fileAddress() + read_u64(24);
}
