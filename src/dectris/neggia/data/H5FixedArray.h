// SPDX-License-Identifier: MIT

#ifndef FIXEDARRAYHEADER_H
#define FIXEDARRAYHEADER_H
#include <dectris/neggia/data/H5Object.h>
#include <string>
#include <vector>

class H5FixedArrayDataBlock : public H5Object {
public:
    H5FixedArrayDataBlock(const char* fileAddress,
                          size_t offset,
                          size_t entrySize,
                          size_t numEntries,
                          size_t numElementsPerPage);

    size_t numElements() const;
    size_t element(int i) const;

private:
    void init();

    size_t _entrySize;
    size_t _numEntries;
    size_t _numElementsPerPage;
    size_t _numPages;
    size_t _pageBitMapSize;
};

class H5FixedArrayHeader : public H5Object {
public:
    H5FixedArrayHeader(const char* fileAddress, size_t offset);
    H5FixedArrayHeader(const H5Object& obj);

    size_t numElements() const;
    size_t element(int i) const;

private:
    H5FixedArrayDataBlock initDataBlock();
    H5FixedArrayDataBlock _dataBlock;
};

#endif  // FIXEDARRAYHEADER_H
