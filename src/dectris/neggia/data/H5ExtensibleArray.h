// SPDX-License-Identifier: MIT

#ifndef EXTENSIBLEARRAYHEADER_H
#define EXTENSIBLEARRAYHEADER_H
#include <dectris/neggia/data/H5Object.h>
#include <string>
#include <vector>

class H5ExtensibleArrayIndexBlock : public H5Object {
public:
    H5ExtensibleArrayIndexBlock(const char* fileAddress,
                                size_t offset,
                                size_t numElements,
                                size_t numElementsInIndexBlock,
                                size_t elementSize);

    size_t numElements() const;
    size_t element(int i) const;

private:
    void init();

    size_t _numElements;
    size_t _numElementsInIndexBlock;
    size_t _elementSize;
};

class H5ExtensibleArrayHeader : public H5Object {
public:
    H5ExtensibleArrayHeader(const char* fileAddress, size_t offset);
    H5ExtensibleArrayHeader(const H5Object& obj);

    size_t numElements() const;
    size_t element(int i) const;

private:
    H5ExtensibleArrayIndexBlock initIndexBlock();

    H5ExtensibleArrayIndexBlock _indexBlock;
};

#endif  // EXTENSIBLEARRAYHEADER_H
