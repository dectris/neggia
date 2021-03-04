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