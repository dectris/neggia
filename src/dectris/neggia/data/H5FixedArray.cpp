// SPDX-License-Identifier: MIT

#include "H5FixedArray.h"
#include <assert.h>
#include <cmath>
#include <map>
#include "JenkinsLookup3Checksum.h"

#ifdef DEBUG_PARSING
#include <iostream>
#endif

H5FixedArrayHeader::H5FixedArrayHeader(const char* fileAddress, size_t offset)
      : H5Object(fileAddress, offset), _dataBlock(initDataBlock()) {}

H5FixedArrayHeader::H5FixedArrayHeader(const H5Object& obj)
      : H5Object(obj), _dataBlock(initDataBlock()) {}

H5FixedArrayDataBlock H5FixedArrayHeader::initDataBlock() {
    std::string signature = std::string(address(), 4);
    assert(signature == "FAHD");
    uint32_t checksum = read_u32(24);
    uint32_t checkSumCalculated =
            JenkinsLookup3Checksum(std::string(address(), 24));
    assert(checksum == checkSumCalculated);

    uint8_t version = read_u8(4);
    assert(version == 0);
    uint8_t clientID = read_u8(5);
    uint8_t entrySize = read_u8(6);
    uint8_t pageBits = read_u8(7);
    size_t numEntries = read_u64(8);
    size_t dataBlockAddress = read_u64(16);

#ifdef DEBUG_PARSING
    std::cerr << "  Fixed Array Header\n";
    std::cerr << "    version:   " << (int)version << "\n";
    std::cerr << "    client id: " << (int)clientID << "\n";
    std::cerr << "    entry size: " << (int)entrySize << "\n";
    std::cerr << "    page bits: " << (int)pageBits << "\n";
    std::cerr << "    max num entries: " << numEntries << "\n";
#endif

    // see hdf5 source for paging implementation:
    // https://github.com/HDFGroup/hdf5/blob/develop/src/H5FAdblock.c#L110
    size_t numElementsPerPage = (size_t)1 << pageBits;

    H5FixedArrayDataBlock dataBlock(fileAddress(), dataBlockAddress, entrySize,
                                    numEntries, numElementsPerPage);
    assert(dataBlock.read_u64(6) == offset());  // consistency check
    return dataBlock;
}

size_t H5FixedArrayHeader::numElements() const {
    return _dataBlock.numElements();
}

size_t H5FixedArrayHeader::element(int i) const {
    return _dataBlock.element(i);
}

H5FixedArrayDataBlock::H5FixedArrayDataBlock(const char* fileAddress,
                                             size_t offset,
                                             size_t entrySize,
                                             size_t numEntries,
                                             size_t numElementsPerPage)
      : H5Object(fileAddress, offset),
        _entrySize(entrySize),
        _numEntries(numEntries),
        _numElementsPerPage(numElementsPerPage),
        _numPages(0),
        _pageBitMapSize(0) {
    std::string signature = std::string(address(), 4);
    assert(signature == "FADB");

    size_t entriesInDataBlock = numEntries;
    if (numEntries > numElementsPerPage) {
        _numPages =
                ((numEntries + numElementsPerPage) - 1) / numElementsPerPage;
        entriesInDataBlock = 0;
        _pageBitMapSize = (_numPages + 7) / 8;
    }
    size_t dataBlockChecksumOffset =
            14 + _pageBitMapSize + entriesInDataBlock * entrySize;
    uint32_t checksum = read_u32(dataBlockChecksumOffset);
    uint32_t checkSumCalculated = JenkinsLookup3Checksum(
            std::string(address(), dataBlockChecksumOffset));
    assert(checksum == checkSumCalculated);
}

size_t H5FixedArrayDataBlock::numElements() const {
    return _numEntries;
}

size_t H5FixedArrayDataBlock::element(int i) const {
    if (_numPages == 0) {
        return read_u64(14 + i * _entrySize);
    }
    size_t pageIndex = i / _numElementsPerPage;
    size_t pageSize = (_numElementsPerPage * _entrySize) + 4;
    size_t pageStart = 14 /*header*/ + _pageBitMapSize + 4 /*checksum*/ +
                       pageIndex * pageSize;

    if (pageIndex == _numPages - 1) {
        if (_numEntries % _numElementsPerPage != 0) {
            pageSize = (_numEntries % _numElementsPerPage) * _entrySize + 4;
        }
    }

    uint32_t checksumPage = read_u32(pageStart + pageSize - 4);
    uint32_t checkSumCalculated = JenkinsLookup3Checksum(
            std::string(address() + pageStart, pageSize - 4));

    assert(checksumPage == checkSumCalculated);
    size_t elementIndexInPage = i % _numElementsPerPage;
    return read_u64(pageStart + elementIndexInPage * _entrySize);
}