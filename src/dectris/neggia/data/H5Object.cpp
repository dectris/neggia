// SPDX-License-Identifier: MIT

#include "H5Object.h"
#include <assert.h>
#include <string.h>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include "constants.h"

H5Object::H5Object() : _fileAddress(nullptr), _offset(0) {}

H5Object::H5Object(const char* address, size_t offset)
      : _fileAddress(address), _offset(offset) {
    if (_offset == H5_INVALID_ADDRESS)
        throw std::out_of_range("object pointing to invalid address");
}

size_t H5Object::readIntegerAt(size_t offset, size_t length) const {
    assert(length <= sizeof(size_t));
    size_t returnValue = 0;
    memcpy(&returnValue, address() + offset, length);
    return returnValue;
}

void H5Object::debugPrint(int nBytes) const {
    std::cout << "OFFSET: " << _offset << std::endl;
    for (int i = 0; i < nBytes; ++i) {
        std::cout << std::hex << std::setfill('0') << std::setw(2)
                  << (unsigned int)(*(unsigned char*)(_fileAddress + _offset +
                                                      i))
                  << " ";
        if ((i + 1) % 4 == 0) {
            std::cout << std::dec << " " << (i - 3) << std::endl;
        }
    }
    std::cout << std::endl;
}
