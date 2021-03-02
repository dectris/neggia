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

#ifndef DATASET_H
#define DATASET_H

#include <dectris/neggia/data/H5DataLayoutMsg.h>
#include <dectris/neggia/data/H5Path.h>
#include <dectris/neggia/data/H5SymbolTableEntry.h>
#include <memory>
#include <string>
#include <vector>
#include "H5File.h"

class H5LinkMsg;
class H5LinkInfoMsg;

class Dataset {
public:
    Dataset();
    Dataset(const H5File& h5File, const std::string& path);
    ~Dataset();

    unsigned int dataTypeId() const;
    size_t dataSize() const;
    bool isSigned() const;
    std::vector<size_t> dim() const;
    bool isChunked() const;
    std::vector<size_t> chunkSize() const;

    // chunkOffset is ignored for contigous or raw datasets
    void read(void* data,
              const std::vector<size_t>& chunkOffset =
                      std::vector<size_t>()) const;

private:
    struct ConstDataPointer {
        const char* data;
        size_t size;
    };

    void parseDataSymbolTable();
    void readRawData(ConstDataPointer rawData,
                     void* outData,
                     size_t outDataSize) const;
    void readLz4Data(ConstDataPointer rawData, void* data, size_t s) const;
    void readBitshuffleData(ConstDataPointer rawData,
                            void* data,
                            size_t s) const;
    size_t getSizeOfOutData() const;
    H5Object dataChunkFromObjectHeader(const H5ObjectHeader& objHeader,
                                       const std::vector<size_t>& offset) const;
    ConstDataPointer getRawData(const std::vector<size_t>& chunkOffset) const;

    H5File _h5File;
    H5ObjectHeader _dataSymbolObjectHeader;
    H5DataLayoutMsg _dataLayoutMsg;
    std::vector<size_t> _dim;
    bool _isChunked;
    std::vector<size_t> _chunkSize;
    int _filterId;
    std::vector<int32_t> _filterCdValues;
    size_t _dataSize;
    int _dataTypeId;
    bool _isSigned;
};

#endif  // DATASET_H
