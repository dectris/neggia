// SPDX-License-Identifier: MIT

#include "H5ExtensibleArray.h"
#include <assert.h>
#include <cmath>
#include "JenkinsLookup3Checksum.h"

#include <iostream>
#include <map>

#ifdef DEBUG_PARSING
#include <iostream>
#endif

H5ExtensibleArrayHeader::H5ExtensibleArrayHeader(const char* fileAddress,
                                                 size_t offset)
      : H5Object(fileAddress, offset), _indexBlock(initIndexBlock()) {}

H5ExtensibleArrayHeader::H5ExtensibleArrayHeader(const H5Object& obj)
      : H5Object(obj), _indexBlock(initIndexBlock()) {}

H5ExtensibleArrayIndexBlock H5ExtensibleArrayHeader::initIndexBlock() {
    std::string signature = std::string(address(), 4);
    assert(signature == "EAHD");
    uint8_t version = read_u8(4);
    assert(version == 0);
    uint8_t clientID = read_u8(5);
    uint32_t checksum = read_u32(68);
    const uint32_t checkSumCalculated =
            JenkinsLookup3Checksum(std::string(address(), 68));
    assert(checksum == checkSumCalculated);
    uint8_t elementSize = read_u8(6);
    uint8_t maxNumElementsBits = read_u8(7);
    uint8_t numElementsInIndexBlock = read_u8(8);
    uint8_t minNumElementsInDataBlock = read_u8(9);
    uint8_t minNumPtrsInSecondaryBlock = read_u8(10);
    uint8_t maxNumElementsInDataBlockBits = read_u8(11);
    uint64_t numSecondaryBlocks = read_u64(12);
    uint64_t secondaryBlockSize = read_u64(20);
    uint64_t numDataBlocks = read_u64(28);
    uint64_t dataBlockSize = read_u64(36);
    uint64_t maxIndexSet = read_u64(44);
    uint64_t numElementsRealized = read_u64(52);
    size_t indexBlockAddress = read_u64(60);

#ifdef DEBUG_PARSING
    std::cerr << "  Extensible Array Header\n";
    std::cerr << "    version:   " << (int)version << "\n";
    std::cerr << "    client id: " << (int)clientID << "\n";
    std::cerr << "    element size: " << (int)elementSize << "\n";
    std::cerr << "    bits max num elements: " << (int)maxNumElementsBits
              << "\n";
    std::cerr << "    num elements in index block: "
              << (int)numElementsInIndexBlock << "\n";
    std::cerr << "    min num elements in data block: "
              << (int)minNumElementsInDataBlock << "\n";
    std::cerr << "    min num ptrs in sec. block: "
              << (int)minNumPtrsInSecondaryBlock << "\n";
    std::cerr << "    bits max num elements in data block: "
              << (int)maxNumElementsInDataBlockBits << "\n";
    std::cerr << "    num secondary blocks: " << numSecondaryBlocks << "\n";
    std::cerr << "    size secondary blocks: " << secondaryBlockSize << "\n";
    std::cerr << "    num data blocks: " << numDataBlocks << "\n";
    std::cerr << "    size data blocks: " << dataBlockSize << "\n";
    std::cerr << "    max index set: " << maxIndexSet << "\n";
    std::cerr << "    num elements realized: " << numElementsRealized << "\n";
#endif

    // hdf5 library code:
    // https://github.com/HDFGroup/hdf5/blob/develop/src/H5EA.c#L312

    H5ExtensibleArrayIndexBlock indexBlock(
            fileAddress(), indexBlockAddress, numElementsInIndexBlock,
            numElementsInIndexBlock, elementSize);
    assert(indexBlock.read_u64(6) == offset());  // consistency check
    return indexBlock;
}

size_t H5ExtensibleArrayHeader::numElements() const {
    return _indexBlock.numElements();
}
size_t H5ExtensibleArrayHeader::element(int i) const {
    return _indexBlock.element(i);
}

size_t H5ExtensibleArrayIndexBlock::numElements() const {
    return _numElements;
}

size_t H5ExtensibleArrayIndexBlock::element(int i) const {
    if (i < _numElementsInIndexBlock) {
        return read_u64(14 + i * _elementSize);
    }
    throw std::runtime_error("elements outside of indexblock not implemented.");
}

H5ExtensibleArrayIndexBlock::H5ExtensibleArrayIndexBlock(
        const char* fileAddress,
        size_t offset,
        size_t numElements,
        size_t numElementsInIndexBlock,
        size_t elementSize)
      : H5Object(fileAddress, offset),
        _numElements(numElements),
        _numElementsInIndexBlock(numElementsInIndexBlock),
        _elementSize(elementSize) {
    std::string signature = std::string(address(), 4);
    assert(signature == "EAIB");
}
