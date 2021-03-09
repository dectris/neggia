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
#include <stdexcept>
#include "H5ObjectHeader.h"
#include "H5SymbolTableEntry.h"
#include "PathResolverV0.h"
#include "PathResolverV2.h"
#include "constants.h"

H5Superblock::H5Superblock(const char* fileAddress) : H5Object(fileAddress, 0) {
    const char magicNumber[] = "\211HDF\r\n\032\n";
    assert(std::string(fileAddress, 8) == std::string(magicNumber));
}

uint8_t H5Superblock::version() const {
    return read_u8(8);
}

ResolvedPath H5Superblock::resolve(const H5Path& path) {
    switch (version()) {
        case 0:
            return resolveV0(path);
        case 2:
            return resolveV2(path);
        default:
            throw(std::runtime_error("superblock version " +
                                     std::to_string(version()) +
                                     " not supported."));
    }
}

ResolvedPath H5Superblock::resolveV0(const H5Path& path) {
    // verify header information
    int version = (int)fileAddress()[8];
    assert(version == 0);
    int offsetSize = (int)fileAddress()[13];
    assert(offsetSize == 8);
    int offsetLength = (int)fileAddress()[14];
    assert(offsetLength == 8);
    assert(fileAddress()[15] == 0);
    uint64_t baseAddress = *(uint64_t*)(fileAddress() + 24);
    assert(baseAddress == 0);
    uint64_t DriverInformationBlockAddress =
            *(uint64_t*)(fileAddress() + 24 + 3 * offsetSize);
    assert(DriverInformationBlockAddress == H5_INVALID_ADDRESS);
    return PathResolverV0(H5SymbolTableEntry(at(24 + 4 * 8))).resolve(path);
}

ResolvedPath H5Superblock::resolveV2(const H5Path& path) {
    // verify header information
    int version = (int)fileAddress()[8];
    assert(version == 2);
    int offsetSize = (int)fileAddress()[9];
    assert(offsetSize == 8);
    int offsetLength = (int)fileAddress()[10];
    assert(offsetLength == 8);
    uint64_t baseAddress = *(uint64_t*)(fileAddress() + 12);
    assert(baseAddress == 0);
    uint64_t extensionAddress = *(uint64_t*)(fileAddress() + 20);
    assert(extensionAddress == H5_INVALID_ADDRESS);
    uint64_t rootGroupHeaderOffset = *(uint64_t*)(fileAddress() + 36);
    return PathResolverV2(H5ObjectHeader(fileAddress(), rootGroupHeaderOffset))
            .resolve(path);
}
