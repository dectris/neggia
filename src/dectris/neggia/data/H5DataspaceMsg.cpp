// SPDX-License-Identifier: MIT

#include "H5DataspaceMsg.h"
#include <stdexcept>
#include "assert.h"

H5DataspaceMsg::H5DataspaceMsg(const H5Object& obj) : H5Object(obj) {
    this->_init();
}

H5DataspaceMsg::H5DataspaceMsg(const char* fileAddress, size_t offset)
      : H5Object(fileAddress, offset) {
    this->_init();
}

uint8_t H5DataspaceMsg::version() const {
    return this->read_u8(0);
}

uint8_t H5DataspaceMsg::rank() const {
    return this->read_u8(1);
}

bool H5DataspaceMsg::maxDims() const {
    return this->read_u8(2) & 0x1;
}

uint64_t H5DataspaceMsg::dim(int i) const {
    return this->read_u64(_dimsOffset + i * 8);
}

uint64_t H5DataspaceMsg::maxDim(int i) const {
    assert(this->maxDims());
    return this->read_u64(_dimsOffset + this->rank() * 8 + i * 8);
}

void H5DataspaceMsg::_init() {
    switch (version()) {
        case 1:
            _dimsOffset = 8;
            break;
        case 2:
            _dimsOffset = 4;
            break;
        default:
            throw std::runtime_error("Dataspace Message version " +
                                     std::to_string(version()) +
                                     " not supported.");
    }
}
