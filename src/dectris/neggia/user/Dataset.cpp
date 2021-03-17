// SPDX-License-Identifier: MIT

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
    H5Superblock root(_h5File.fileAddress());
    try {
        auto resolvedPath = root.resolve(path);
        while (resolvedPath.externalFile) {
            auto targetFile = resolvedPath.externalFile->filename;
            if (targetFile[0] != '/')
                targetFile = _h5File.fileDir() + "/" + targetFile;
            _h5File = H5File(targetFile);
            root = H5Superblock(_h5File.fileAddress());
            resolvedPath = root.resolve(resolvedPath.externalFile->h5Path);
        }
        _dataSymbolObjectHeader = resolvedPath.objectHeader;
    } catch (std::exception& exc) {
        std::cerr << "exception during path resolve: " << exc.what() << "\n";
        throw std::out_of_range(exc.what());
    }
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
    if (outDataSize != rawData.size) {
        throw std::runtime_error("cannot read " + std::to_string(outDataSize) +
                                 " bytes from a dataset of size " +
                                 std::to_string(rawData.size));
    }
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
    if (_isChunked) {
        assert(_chunkSize == std::vector<size_t>({1, _dim[1], _dim[2]}));
        return _dataSize * _dim[1] * _dim[2];
    }
    for (auto d : _dim)
        s *= d;
    return s;
}

namespace {
template <class T1, class T2>
bool chunkCompareGreaterEqual(const T1* key0, const T2* key1, size_t len) {
    for (ssize_t idx = ((ssize_t)len) - 1; idx >= 0; --idx) {
        if (key0[idx] < key1[idx])
            return false;
        if (key0[idx] > key1[idx])
            return true;
    }
    return true;
}
}  // namespace

H5Object Dataset::dataChunkFromObjectHeader(
        const H5ObjectHeader& objHeader,
        const std::vector<size_t>& offset) const {
    const size_t keySize = 8 + offset.size() * 8;
    const size_t childSize = 8;

    for (int i = 0; i < objHeader.numberOfMessages(); ++i) {
        H5HeaderMessage msg(objHeader.headerMessage(i));
        if (msg.type == H5DataLayoutMsg::TYPE_ID) {
            H5DataLayoutMsg dataLayoutMsg(msg.object);
            H5BLinkNode bTree(dataLayoutMsg.chunkBTree());

            while (bTree.nodeLevel() > 0) {
                bool found = false;
                for (int i = bTree.entriesUsed() - 1; i >= 0; --i) {
                    H5Object key(bTree + 24 + i * (keySize + childSize));
                    if (chunkCompareGreaterEqual(
                                offset.data(), (const uint64_t*)key.address(8),
                                offset.size()))
                    {
                        bTree = H5BLinkNode(key.fileAddress(),
                                            key.read_u64(keySize));
                        found = true;
                        break;
                    }
                }
                if (!found)
                    throw std::runtime_error("Not found");
            }

            for (int i = 0; i < bTree.entriesUsed(); ++i) {
                H5Object key(bTree + 24 + i * (keySize + childSize));
                if (memcmp(key.address(8), offset.data(),
                           offset.size() * sizeof(uint64_t)) == 0)
                {
                    return key;
                }
            }
            throw std::runtime_error("Not found");
        }
    }
    throw std::runtime_error("Missing H5DataCache Layout");
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
        H5Object dataChunk(dataChunkFromObjectHeader(_dataSymbolObjectHeader,
                                                     chunkOffsetFullSize));
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
        case -1:
            readRawData(rawData, data, s);
            break;
        case LZ4_FILTER:
            readLz4Data(rawData, data, s);
            break;
        case BSHUF_H5FILTER:
            readBitshuffleData(rawData, data, s);
            break;
        default:
            throw std::runtime_error("Unknown filter");
    }
}

void Dataset::parseDataSymbolTable() {
    for (int i = 0; i < _dataSymbolObjectHeader.numberOfMessages(); ++i) {
        H5HeaderMessage msg(_dataSymbolObjectHeader.headerMessage(i));
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
