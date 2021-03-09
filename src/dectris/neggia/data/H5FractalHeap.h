// SPDX-License-Identifier: MIT

#include <dectris/neggia/data/H5Object.h>

class H5FractalHeap : public H5Object {
public:
    H5FractalHeap();
    H5FractalHeap(const char* fileAddress, size_t offset);
    H5FractalHeap(const H5Object&);
    H5Object getHeapObject(size_t offset) const;

private:
    size_t getBlockOffsetSize() const;
    uint64_t getFiltersEncodedLength() const;
    bool filtersArePresent() const;
    bool blocksAreChecksummed() const;
    uint16_t getTableWidth() const;
    size_t getMaximumNumberOfDirectBlocks() const;
    size_t getStartingBlockSize() const;
    size_t getMaximumDirectBlockSize() const;
    size_t getRow(size_t offset) const;
    size_t getRowOffset(size_t row) const;
    size_t getBlockSize(size_t row) const;
    H5Object getHeapObjectInDirectBlock(const H5Object& directBlock,
                                        size_t heapOffset) const;
    H5Object getHeapObjectInIndirectBlock(const H5Object& indirectBlock,
                                          size_t heapOffset) const;
};
