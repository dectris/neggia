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

#include "H5FilterMsg.h"
#include <assert.h>
#include <stdexcept>

H5FilterMsg::H5FilterMsg(const char* fileAddress, size_t offset)
      : H5Object(fileAddress, offset) {
    this->_init();
}

H5FilterMsg::H5FilterMsg(const H5Object& obj) : H5Object(obj) {
    this->_init();
}

uint8_t H5FilterMsg::version() const {
    return read_u8(0);
}

unsigned int H5FilterMsg::nFilters() const {
    return read_u8(1);
}

uint16_t H5FilterMsg::filterId(int i) const {
    return _filters.at(i).id;
}

std::string H5FilterMsg::filterName(int i) const {
    return _filters.at(i).name;
}

std::vector<int32_t> H5FilterMsg::clientData(int i) const {
    return _filters.at(i).client_data;
}

void H5FilterMsg::_init() {
    switch (version()) {
        case 1:
            _initV1();
            break;
        default:
            throw std::runtime_error("Filter Pipeline Message version " +
                                     std::to_string(version()) +
                                     " not supported.");
    }
}

void H5FilterMsg::_initV1() {
    assert(read_u16(2) == 0);
    assert(read_u32(4) == 0);
    size_t currentOffset = 8;
    for (size_t fId = 0; fId < nFilters(); ++fId) {
        uint16_t filterId = read_u16(currentOffset);
        uint16_t nameLength = read_u16(currentOffset + 2);
        uint16_t nClientValues = read_u16(currentOffset + 6);
        std::string name;
        if (nameLength > 0) {
            name = std::string(address() + currentOffset + 8, nameLength);
        }
        int32_t* array = (int32_t*)address(currentOffset + 8 + nameLength +
                                           (nameLength % 8));
        auto client_data = std::vector<int32_t>(array, array + nClientValues);
        _filters.push_back({filterId, name, client_data});
        currentOffset += 8 + nameLength + (nameLength % 8) + 4 * nClientValues +
                         (nClientValues % 2) * 4;
    }
}