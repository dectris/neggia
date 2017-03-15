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

#include <dectris/neggia/data/H5Object.h>

class H5FractalHeap: public H5Object {
public:
    H5FractalHeap();
    H5FractalHeap(const char * fileAddress, size_t offset);
    H5FractalHeap(const H5Object &);
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
    H5Object getHeapObjectInDirectBlock(const H5Object & directBlock, size_t heapOffset) const;
    H5Object getHeapObjectInIndirectBlock(const H5Object & indirectBlock, size_t heapOffset) const;
};
