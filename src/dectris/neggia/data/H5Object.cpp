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
