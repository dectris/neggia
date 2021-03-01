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

#include "Dataset.h"
#include <assert.h>
#include <dectris/neggia/data/Decode.h>
#include <dectris/neggia/data/H5BTreeVersion2.h>
#include <dectris/neggia/data/H5DataspaceMsg.h>
#include <dectris/neggia/data/H5DatatypeMsg.h>
#include <dectris/neggia/data/H5FilterMsg.h>
#include <dectris/neggia/data/H5FractalHeap.h>
#include <dectris/neggia/data/H5LinkInfoMessage.h>
#include <dectris/neggia/data/H5LinkMsg.h>
#include <dectris/neggia/data/H5LocalHeap.h>
#include <dectris/neggia/data/H5Superblock.h>
#include <dectris/neggia/data/H5SymbolTableEntry.h>
#include <dectris/neggia/data/constants.h>
#include <string.h>
#include <iostream>
#include <sstream>

Dataset::Dataset()
      : _filterId(-1), _dataSize(0), _dataTypeId(-1), _isSigned(false) {}

Dataset::Dataset(const H5File& h5File, const std::string& path)
      : _h5File(h5File),
        _filterId(-1),
        _dataSize(0),
        _dataTypeId(-1),
        _isSigned(false) {
    H5SymbolTableEntry root =
            H5Superblock(_h5File.fileAddress()).rootGroupSymbolTableEntry();
    resolvePath(root, path);
    parseDataSymbolTable();
}

Dataset::~Dataset() {}

unsigned int Dataset::dataTypeId() const {
    return (unsigned int)_dataTypeId;
}

size_t Dataset::dataSize() const {
    return _dataSize;
}

bool Dataset::isSigned() const {
    return _isSigned;
}

std::vector<size_t> Dataset::dim() const {
    return _dim;
}

bool Dataset::isChunked() const {
    return _isChunked;
}

std::vector<size_t> Dataset::chunkSize() const {
    return _chunkSize;
}

void Dataset::readRawData(ConstDataPointer rawData,
                          void* outData,
                          size_t outDataSize) const {
    assert(outDataSize == rawData.size);
    memcpy(outData, rawData.data, outDataSize);
}

void Dataset::readLz4Data(Dataset::ConstDataPointer rawData,
                          void* data,
                          size_t s) const {
    lz4Decode(rawData.data, (char*)data, s);
}

void Dataset::readBitshuffleData(ConstDataPointer rawData,
                                 void* data,
                                 size_t s) const {
    assert(_filterCdValues.size() > 4);
    assert(_filterCdValues[4] == BSHUF_H5_COMPRESS_LZ4);
    int elementSize = _filterCdValues[2];
    bshufUncompressLz4(rawData.data, (char*)data, s, elementSize);
}

size_t Dataset::getSizeOfOutData() const {
    size_t s = _dataSize;
    for (auto d : _dim)
        s *= d;
    return s;
}

Dataset::ConstDataPointer Dataset::getRawData(
        const std::vector<size_t>& chunkOffset) const {
    const char* rawData = nullptr;
    size_t rawDataSize = 0;
    if (_isChunked) {
        // internally hdf5 stores chunk size with one dimension more than the
        // dimensions of the dataset
        // https://www.hdfgroup.org/HDF5/doc/H5.format.html#LayoutMessage
        std::vector<size_t> chunkOffsetFullSize(chunkOffset);
        while (chunkOffsetFullSize.size() < _chunkSize.size() + 1)
            chunkOffsetFullSize.push_back(0);
        H5Object dataChunk(_dataSymbolTable.dataChunk(chunkOffsetFullSize));
        rawData = dataChunk.fileAddress() +
                  dataChunk.read_u64(8 + chunkOffsetFullSize.size() * 8);
        rawDataSize = dataChunk.read_u32(0);
    } else {
        rawData = _dataLayoutMsg.dataAddress();
        rawDataSize = _dataLayoutMsg.dataSize();
    }
    return ConstDataPointer{rawData, rawDataSize};
}

void Dataset::read(void* data, const std::vector<size_t>& chunkOffset) const {
    auto rawData = getRawData(chunkOffset);
    size_t s = getSizeOfOutData();
    switch (_filterId) {
        case -1: {
            readRawData(rawData, data, s);
        } break;
        case LZ4_FILTER: {
            readLz4Data(rawData, data, s);
        } break;
        case BSHUF_H5FILTER: {
            readBitshuffleData(rawData, data, s);
        } break;
        default:
            throw std::runtime_error("Unknown filter");
    }
}

void Dataset::resolvePath(const H5SymbolTableEntry& in, const H5Path& path) {
    assert(in.cacheType() == H5SymbolTableEntry::DATA ||
           in.cacheType() ==
                   H5SymbolTableEntry::GROUP);  // makes sense only for groups
                                                // (1) or objects that contain
                                                // links via objectheader (cache
                                                // type = 0, 1)
    H5SymbolTableEntry parentEntry(
            path.isAbsolute() ? H5Superblock(_h5File.fileAddress())
                                        .rootGroupSymbolTableEntry()
                              : in);

    for (auto itemIterator = path.begin(); itemIterator != path.end();
         ++itemIterator)
    {
        auto item = *itemIterator;
        if (parentEntry.cacheType() == H5SymbolTableEntry::GROUP) {
            H5SymbolTableEntry stEntry;
            try {
                stEntry = parentEntry.find(item);
            } catch (const std::out_of_range&) {
                findPathInObjectHeader(parentEntry, item,
                                       H5Path(path, itemIterator + 1));
                return;
            }
            if (stEntry.cacheType() == H5SymbolTableEntry::LINK) {
                findPathInScratchSpace(parentEntry, stEntry,
                                       H5Path(path, itemIterator + 1));
                return;
            } else {
                parentEntry = stEntry;
                continue;
            }
        } else {
            throw std::runtime_error(
                    "Expected GROUP (cache_type = 1) at path item " + item);
        }
    }

    _dataSymbolTable = parentEntry;
}

void Dataset::findPathInScratchSpace(H5SymbolTableEntry parentEntry,
                                     H5SymbolTableEntry symbolTableEntry,
                                     const H5Path& remainingPath) {
    size_t targetNameOffset = symbolTableEntry.getOffsetToLinkValue();
    H5LocalHeap treeHeap =
            H5Object(_h5File.fileAddress(), parentEntry.getAddressOfHeap());
    H5Path targetPath(treeHeap.data(targetNameOffset));
    resolvePath(parentEntry, targetPath + remainingPath);
}

void Dataset::findPathInLinkMsg(const H5SymbolTableEntry& parentEntry,
                                const H5LinkMsg& linkMsg,
                                const H5Path& remainingPath) {
    if (linkMsg.linkType() == H5LinkMsg::SOFT) {
        H5Path targetPath(linkMsg.targetPath());
        resolvePath(parentEntry, targetPath + remainingPath);
        return;
    } else if (linkMsg.linkType() == H5LinkMsg::EXTERNAL) {
        std::string targetFile = linkMsg.targetFile();
        if (targetFile[0] != '/')
            targetFile = _h5File.fileDir() + "/" + targetFile;
        _h5File = H5File(targetFile);
        H5Path targetPath(linkMsg.targetPath());
        resolvePath(
                H5Superblock(_h5File.fileAddress()).rootGroupSymbolTableEntry(),
                targetPath + remainingPath);
        return;
    }
}

uint32_t Dataset::getFractalHeapOffset(const H5LinkInfoMsg& linkInfoMsg,
                                       const std::string& pathItem) const {
    size_t btreeAddress = linkInfoMsg.getBTreeAddress();
    if (btreeAddress == H5_INVALID_ADDRESS) {
        throw std::out_of_range("Invalid address");
    }
    H5BTreeVersion2 btree(_h5File.fileAddress(), btreeAddress);
    H5Object heapRecord(_h5File.fileAddress(),
                        btree.getRecordAddress(pathItem));
    return heapRecord.read_u32(5);
}

void Dataset::findPathInObjectHeader(const H5SymbolTableEntry& parentEntry,
                                     const std::string pathItem,
                                     const H5Path& remainingPath) {
    H5ObjectHeader objectHeader = parentEntry.objectHeader();
    for (size_t i = 0; i < objectHeader.numberOfMessages(); ++i) {
        H5HeaderMessage msg(objectHeader.headerMessage(i));
        switch (msg.type) {
            case H5LinkMsg::TYPE_ID: {
                H5LinkMsg linkMsg(msg.object);
                if (linkMsg.linkName() != pathItem)
                    continue;
                findPathInLinkMsg(parentEntry, linkMsg, remainingPath);
                return;
            } break;
            case H5LinkInfoMsg::TYPE_ID: {
                uint32_t heapOffset;
                H5LinkInfoMsg linkInfoMsg(msg.object);
                try {
                    heapOffset = getFractalHeapOffset(linkInfoMsg, pathItem);
                } catch (const std::out_of_range&) {
                    continue;
                }
                H5FractalHeap fractalHeap(_h5File.fileAddress(),
                                          linkInfoMsg.getFractalHeapAddress());
                H5LinkMsg linkMsg(fractalHeap.getHeapObject(heapOffset));
                assert(linkMsg.linkName() == pathItem);
                findPathInLinkMsg(parentEntry, linkMsg, remainingPath);
                return;
            } break;
            default: {
                continue;
            }
        }
    }
    throw std::out_of_range("Not found");
}

void Dataset::parseDataSymbolTable() {
    H5ObjectHeader dataObjectHeader = _dataSymbolTable.objectHeader();
    for (int i = 0; i < dataObjectHeader.numberOfMessages(); ++i) {
        H5HeaderMessage msg(dataObjectHeader.headerMessage(i));
        switch (msg.type) {
            case H5DataspaceMsg::TYPE_ID: {
                H5DataspaceMsg dataspaceMsg(msg.object);
                _dim.clear();
                for (size_t i = 0; i < dataspaceMsg.rank(); ++i) {
                    _dim.push_back(dataspaceMsg.dim(i));
                }
                break;
            }
            case H5DataLayoutMsg::TYPE_ID: {
                _dataLayoutMsg = H5DataLayoutMsg(msg.object);
                switch (_dataLayoutMsg.layoutClass()) {
                    case 0:
                    case 1:
                        _isChunked = false;
                        break;
                    case 2:
                        _isChunked = true;
                        _chunkSize.clear();
                        for (size_t i = 0; i < _dataLayoutMsg.chunkDims() - 1;
                             ++i) {
                            _chunkSize.push_back(_dataLayoutMsg.chunkDim(i));
                        }
                        break;
                    default:
                        assert(false);
                }
                break;
            }
            case H5FilterMsg::TYPE_ID: {
                H5FilterMsg filterMsg(msg.object);
                // We accept at most on filter
                assert(filterMsg.nFilters() <= 1);
                if (filterMsg.nFilters() == 1)
                    _filterId = filterMsg.filterId(0);
                _filterCdValues = filterMsg.clientData(0);
                break;
            }
            case H5DatatypeMsg::TYPE_ID: {
                H5DatatypeMsg datatypeMsg(msg.object);
                _dataSize = datatypeMsg.dataSize();
                _dataTypeId = datatypeMsg.typeId();
                _isSigned = datatypeMsg.isSigned();
                break;
            }
        }
    }
    assert(_dataTypeId >= 0);
    assert(_dataSize > 0);
}
