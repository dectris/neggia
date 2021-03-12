// SPDX-License-Identifier: MIT

#include "H5Superblock.h"
#include <assert.h>
#include <stdexcept>
#include "H5ObjectHeader.h"
#include "H5SymbolTableEntry.h"
#include "PathResolverV0.h"
#include "PathResolverV2.h"
#include "constants.h"
#ifdef DEBUG_PARSING
#include <iostream>
#endif

H5Superblock::H5Superblock(const char* fileAddress) : H5Object(fileAddress, 0) {
    const char magicNumber[] = "\211HDF\r\n\032\n";
    assert(std::string(fileAddress, 8) == std::string(magicNumber));
}

uint8_t H5Superblock::version() const {
    return read_u8(8);
}

ResolvedPath H5Superblock::resolve(const H5Path& path) {
#ifdef DEBUG_PARSING
    std::cerr << ">>> superblock version " << (int)version() << " resolving "
              << std::string(path) << "\n";
#endif
    switch (version()) {
        case 0:
            return resolveV0(path);
        case 2:
        case 3:
            return resolveV2(path);
        default:
            throw std::runtime_error("superblock version " +
                                     std::to_string(version()) +
                                     " not supported.");
    }
}

ResolvedPath H5Superblock::resolveV0(const H5Path& path) {
    // verify header information
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
    int offsetSize = (int)fileAddress()[9];
    assert(offsetSize == 8);
    int offsetLength = (int)fileAddress()[10];
    assert(offsetLength == 8);
    if (version() == 3) {
        // we need to check that the file is not open for write access
        uint8_t fileConsistencyFlags = (uint8_t)fileAddress()[11];
        if ((fileConsistencyFlags & 5) > 0)
            throw std::runtime_error("file opened for write access");
    }
    uint64_t baseAddress = *(uint64_t*)(fileAddress() + 12);
    assert(baseAddress == 0);
    uint64_t extensionAddress = *(uint64_t*)(fileAddress() + 20);
    assert(extensionAddress == H5_INVALID_ADDRESS);
    uint64_t rootGroupHeaderOffset = *(uint64_t*)(fileAddress() + 36);
    return PathResolverV2(H5ObjectHeader(fileAddress(), rootGroupHeaderOffset))
            .resolve(path);
}
