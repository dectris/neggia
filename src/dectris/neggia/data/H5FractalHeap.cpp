// SPDX-License-Identifier: MIT

#include "H5FractalHeap.h"
#include <assert.h>
#include <string.h>
#include <cmath>
#include <iostream>
#include <string>

H5FractalHeap::H5FractalHeap() {}

H5FractalHeap::H5FractalHeap(const char* fileAddress, size_t offset)
      : H5Object(fileAddress, offset) {}

H5FractalHeap::H5FractalHeap(const H5Object& obj) : H5Object(obj) {
    assert(std::string(address(), 4) == "FRHP");
}

size_t H5FractalHeap::getBlockOffsetSize() const {
    uint16_t maximumHeapSize = this->read_u16(128);
    size_t blockOffsetSize = maximumHeapSize / 8;
    if (maximumHeapSize % 8 > 0)
        blockOffsetSize += 1;
    return blockOffsetSize;
}

uint64_t H5FractalHeap::getFiltersEncodedLength() const {
    return this->read_u16(7);
}

bool H5FractalHeap::filtersArePresent() const {
    return getFiltersEncodedLength() > 0;
}

bool H5FractalHeap::blocksAreChecksummed() const {
    uint8_t flags = this->read_u8(9);
    return (bool)(flags & 0x02);
}

uint16_t H5FractalHeap::getTableWidth() const {
    return this->read_u16(110);
}

size_t H5FractalHeap::getMaximumNumberOfDirectBlocks() const {
    return (size_t)(2 + std::log2(getMaximumDirectBlockSize()) -
                    std::log2(getStartingBlockSize())) *
           getTableWidth();
}

size_t H5FractalHeap::getStartingBlockSize() const {
    return this->read_u64(112);
}

size_t H5FractalHeap::getMaximumDirectBlockSize() const {
    return this->read_u64(120);
}

size_t H5FractalHeap::getRow(size_t offset) const {
    auto w = getTableWidth();
    auto s0 = getStartingBlockSize();
    if (offset < w * s0) {
        return 0;
    } else {
        return 1 + (size_t)(std::log2(offset / (w * s0)));
    }
}

size_t H5FractalHeap::getRowOffset(size_t row) const {
    auto w = getTableWidth();
    auto s0 = getStartingBlockSize();
    if (row == 0) {
        return 0;
    } else {
        return w * s0 * std::pow(2, row - 1);
    }
}

size_t H5FractalHeap::getBlockSize(size_t row) const {
    auto s0 = getStartingBlockSize();
    if (row == 0) {
        return s0;
    } else {
        return s0 * std::pow(2, row - 1);
    }
}

H5Object H5FractalHeap::getHeapObjectInDirectBlock(const H5Object& directBlock,
                                                   size_t heapOffset) const {
    assert(std::string(directBlock.address(), 4) == "FHDB");
    return directBlock.at(heapOffset);
}

H5Object H5FractalHeap::getHeapObjectInIndirectBlock(
        const H5Object& indirectBlock,
        size_t heapOffset) const {
    assert(std::string(indirectBlock.address(), 4) == "FHIB");
    size_t row = getRow(heapOffset);
    size_t rowOffset = getRowOffset(row);
    size_t blockSize = getBlockSize(row);
    size_t column = (heapOffset - rowOffset) / blockSize;
    size_t columnOffset = rowOffset + column * blockSize;
    size_t blockNumber = row * getTableWidth() + column;
    size_t maximumNumberOfDirectBlocks = getMaximumNumberOfDirectBlocks();
    if (blockNumber < maximumNumberOfDirectBlocks) {
        size_t blockOffset = 13 + getBlockOffsetSize() +
                             blockNumber * (8 + 16 * filtersArePresent());
        return getHeapObjectInDirectBlock(
                H5Object(fileAddress(), indirectBlock.read_u64(blockOffset)),
                heapOffset - columnOffset);
    } else {
        size_t blockOffset =
                13 + getBlockOffsetSize() +
                maximumNumberOfDirectBlocks * (8 + 16 * filtersArePresent()) +
                8 * (blockNumber - maximumNumberOfDirectBlocks);
        return getHeapObjectInIndirectBlock(
                H5Object(fileAddress(), indirectBlock.read_u64(blockOffset)),
                heapOffset - columnOffset);
    }
}

H5Object H5FractalHeap::getHeapObject(size_t offset) const {
    size_t rootBlockAddress = this->read_i64(132);
    H5Object rootBlock(this->fileAddress(), rootBlockAddress);
    uint16_t currentNRowsInRootIndirectBlock = this->read_u16(140);
    if (currentNRowsInRootIndirectBlock == 0) {  // root block is direct block
        return getHeapObjectInDirectBlock(rootBlock, offset);
    } else {  // root block is indirect block
        return getHeapObjectInIndirectBlock(rootBlock, offset);
    }
}
