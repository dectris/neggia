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

#include "H5LinkInfoMessage.h"

H5LinkInfoMsg::H5LinkInfoMsg(const char* fileAddress, size_t offset)
      : H5Object(fileAddress, offset) {}

H5LinkInfoMsg::H5LinkInfoMsg(const H5Object& other) : H5Object(other) {}

uint8_t H5LinkInfoMsg::getFlags() const {
    return read_u8(1);
}

bool H5LinkInfoMsg::existsMaximumCreationIndex() const {
    return (bool)(getFlags() & 1);
}

uint64_t H5LinkInfoMsg::getFractalHeapAddress() const {
    if (existsMaximumCreationIndex())
        return read_u64(10);
    else
        return read_u64(2);
}

uint64_t H5LinkInfoMsg::getBTreeAddress() const {
    if (existsMaximumCreationIndex())
        return read_u64(18);
    else
        return read_u64(10);
}
