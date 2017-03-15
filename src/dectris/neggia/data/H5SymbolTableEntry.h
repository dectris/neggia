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

#ifndef H5SYMBOLTABLEENTRY_H
#define H5SYMBOLTABLEENTRY_H
#include "H5ObjectHeader.h"
#include <string>
#include <vector>

/// See https://www.hdfgroup.org/HDF5/doc/H5.format.html#SymbolTableEntry

class H5SymbolTableEntry : public H5Object
{
public:
    enum CACHE_TYPE { DATA=0, GROUP=1, LINK=2};

    H5SymbolTableEntry() = default;
    H5SymbolTableEntry(const char * fileAddress, size_t offset);
    H5SymbolTableEntry(const H5Object & other);
    size_t linkNameOffset() const;
    H5ObjectHeader objectHeader() const;
    CACHE_TYPE cacheType() const;
    H5Object scratchSpace() const;
    uint64_t getAddressOfBTree() const;
    uint64_t getAddressOfHeap() const;
    uint32_t getOffsetToLinkValue() const;
    H5SymbolTableEntry find(const std::string & entry) const;
    H5Object dataChunk(const std::vector<size_t> &offset) const;
};

#endif // H5SYMBOLTABLEENTRY_H
