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

#include "H5Superblock.h"
#include <assert.h>
#include "H5SymbolTableEntry.h"
#include "constants.h"

H5Superblock::H5Superblock(const char* fileAddress) : H5Object(fileAddress, 0) {
    const char magicNumber[] = "\211HDF\r\n\032\n";
    assert(std::string(fileAddress, 8) == std::string(magicNumber));
    int version = (int)fileAddress[8];
    assert(version == 0);
    int offsetSize = (int)fileAddress[13];
    assert(offsetSize == 8);
    int offsetLength = (int)fileAddress[14];
    assert(offsetLength == 8);
    assert(fileAddress[15] == 0);
    uint64_t baseAddress = *(uint64_t*)(fileAddress + 24);
    assert(baseAddress == 0);
    uint64_t DriverInformationBlockAddress =
            *(uint64_t*)(fileAddress + 24 + 3 * offsetSize);
    assert(DriverInformationBlockAddress == H5_INVALID_ADDRESS);
}

int H5Superblock::groupLeafNodeK() const {
    return read_u16(16);
}

int H5Superblock::groupInternalNodeK() const {
    return read_u16(18);
}

H5SymbolTableEntry H5Superblock::rootGroupSymbolTableEntry() const {
    return H5SymbolTableEntry(at(24 + 4 * 8));
}
