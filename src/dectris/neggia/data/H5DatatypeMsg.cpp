// SPDX-License-Identifier: MIT

#include "H5DatatypeMsg.h"
#include <assert.h>

H5DatatypeMsg::H5DatatypeMsg(const char* fileAddress, size_t offset)
      : H5Object(fileAddress, offset) {
    this->_init();
}

H5DatatypeMsg::H5DatatypeMsg(const H5Object& obj) : H5Object(obj) {
    this->_init();
}

unsigned int H5DatatypeMsg::version() const {
    return (this->read_u8(0) & 0xf0) >> 4;
}

unsigned int H5DatatypeMsg::typeId() const {
    return this->read_u8(0) & 0x0f;
}

unsigned int H5DatatypeMsg::dataSize() const {
    return this->read_u32(4);
}

bool H5DatatypeMsg::isSigned() const {
    if (typeId() == 0)
        return this->read_u8(1) & 0x8;
    else if (typeId() == 1)
        return true;
}

void H5DatatypeMsg::_init() {
    unsigned int t = typeId();
    assert(t == 0 || t == 1);
}
