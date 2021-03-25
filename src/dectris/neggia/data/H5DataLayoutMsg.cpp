// SPDX-License-Identifier: MIT

#include "H5DataLayoutMsg.h"
#include <assert.h>
#include <string.h>
#include <iostream>
#include <stdexcept>
#include "H5BTreeVersion2.h"
#include "H5ExtensibleArray.h"
#include "H5FixedArray.h"
#include "JenkinsLookup3Checksum.h"

#define DEBUG_OFFSET 0

H5DataLayoutMsg::H5DataLayoutMsg(const char* fileAddress, size_t offset)
      : H5Object(fileAddress, offset) {
    this->_init();
}

H5DataLayoutMsg::H5DataLayoutMsg(const H5Object& obj) : H5Object(obj) {
    this->_init();
}

uint8_t H5DataLayoutMsg::version() const {
    return this->read_u8(0);
}

uint8_t H5DataLayoutMsg::layoutClass() const {
    return this->read_u8(1);
}

size_t H5DataLayoutMsg::dataSize() const {
    switch (layoutClass()) {
        case 0:
            return read_u16(2);
        case 1:
            return read_u64(2 + 8);
        default:
            throw std::runtime_error("wrong layout class");
    }
}

const char* H5DataLayoutMsg::dataAddress() const {
    switch (layoutClass()) {
        case 0:
            return address() + 2 + 2;
        case 1:
            return fileAddress() + read_u64(2);
        default:
            throw std::runtime_error("wrong layout class");
    }
}

uint8_t H5DataLayoutMsg::chunkDims() const {
    assert(layoutClass() == 2);
    switch (version()) {
        case 3:
            return this->read_u8(2);
        case 4:
            return this->read_u8(3);
        default:
            throw std::runtime_error("Data Layout Message version " +
                                     std::to_string((int)version()) +
                                     " not supported for chunked storage.");
    }
}

size_t H5DataLayoutMsg::dimensionSize() const {
    if (version() < 4) {
        return 4;
    }
    return this->read_u8(4);
}

uint8_t H5DataLayoutMsg::chunkIndexingType() const {
    assert(version() == 4);
    return this->read_u8(5 + dimensionSize() * chunkDims());
}

uint32_t H5DataLayoutMsg::chunkDim(int i) const {
    assert(layoutClass() == 2);
    switch (version()) {
        case 3:
            return read_u32(11 + dimensionSize() * i);
        case 4:
            switch (dimensionSize()) {
                case 1:
                    return this->read_u8(5 + i);
                case 2:
                    return this->read_u16(5 + 2 * i);
                case 4:
                    return this->read_u32(5 + 4 * i);
                default:
                    throw std::runtime_error(
                            "Data Layout Message: dimension size of " +
                            std::to_string(dimensionSize()) +
                            " not supported.");
            }
        default:
            throw std::runtime_error("Data Layout Message version " +
                                     std::to_string((int)version()) +
                                     " not supported.");
    }
}

void H5DataLayoutMsg::_init() {
    assert(version() == 3 || version() == 4);
    switch (layoutClass()) {
        case 0:
        case 1:
            _isChunked = false;
            break;
        case 2:
            _isChunked = true;
            _chunkShape.clear();
            for (size_t i = 0; i < chunkDims() - 1; ++i) {
                _chunkShape.push_back(chunkDim(i));
            }
            break;
        default:
            throw std::runtime_error("Data Layout Message layout class " +
                                     std::to_string((int)layoutClass()) +
                                     " not supported.");
    }
}

H5DataLayoutMsg::ConstDataPointer H5DataLayoutMsg::getRawData(
        size_t elementSize,
        const std::vector<size_t>& chunkOffset) const {
    const char* rawData = nullptr;
    size_t rawDataSize = 0;
    if (_isChunked) {
        switch (version()) {
            case 3:
                return chunkedDataV3(chunkOffset);
            case 4:
                return chunkedDataV4(elementSize, chunkOffset);
            default:
                throw std::runtime_error("Data Layout Message version " +
                                         std::to_string((int)version()) +
                                         " not supported.");
        }
    }
    return ConstDataPointer{dataAddress(), dataSize()};
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

H5DataLayoutMsg::ConstDataPointer H5DataLayoutMsg::chunkedDataV3(
        const std::vector<size_t>& chunkOffset) const {
    // internally hdf5 stores chunk size with one dimension more than the
    // dimensions of the dataset
    // https://support.hdfgroup.org/HDF5/doc/H5.format.html#LayoutMessage
    std::vector<size_t> chunkOffsetFullSize(chunkOffset);
    while (chunkOffsetFullSize.size() < _chunkShape.size() + 1)
        chunkOffsetFullSize.push_back(0);
    H5Object dataChunk(extractDataChunk(chunkOffsetFullSize));
    const char* rawData =
            dataChunk.fileAddress() +
            dataChunk.read_u64(8 + chunkOffsetFullSize.size() * 8);
    size_t rawDataSize = dataChunk.read_u32(0);
    return ConstDataPointer{rawData, rawDataSize};
}

H5DataLayoutMsg::ConstDataPointer H5DataLayoutMsg::chunkedDataV4(
        size_t elementSize,
        const std::vector<size_t>& chunkOffset) const {
    assert(_chunkShape.size() == chunkOffset.size());
    assert(_chunkShape[0] == 1);
    const char* rawData;
    size_t rawDataSize = _chunkShape[1] * _chunkShape[2] * elementSize;
    switch (chunkIndexingType()) {
        case 3: {
            rawData = addressFromFixedArrayStorage(chunkOffset);
            break;
        }
        case 4: {
            rawData = addressFromExtensibleArrayStorage(chunkOffset);
            break;
        }
        case 5: {
            rawData = addressFromBTreeV2Storage(chunkOffset);
            break;
        }
        default:
            throw std::runtime_error("chunk indexing type " +
                                     std::to_string((int)chunkIndexingType()) +
                                     " not supported.");
    }
    return ConstDataPointer{rawData, rawDataSize};
}

const char* H5DataLayoutMsg::addressFromExtensibleArrayStorage(
        const std::vector<size_t>& chunkOffset) const {
    size_t headerAddress = read_u64(11 + dimensionSize() * chunkDims());
    H5ExtensibleArrayHeader header(fileAddress(), headerAddress);
    assert(chunkOffset[0] < header.numElements());
    return fileAddress() + header.element(chunkOffset[0]);
}

const char* H5DataLayoutMsg::addressFromFixedArrayStorage(
        const std::vector<size_t>& chunkOffset) const {
    size_t headerAddress = read_u64(7 + dimensionSize() * chunkDims());
    H5FixedArrayHeader header(fileAddress(), headerAddress);
    assert(chunkOffset[0] < header.numElements());
    return fileAddress() + header.element(chunkOffset[0]);
}

const char* H5DataLayoutMsg::addressFromBTreeV2Storage(
        const std::vector<size_t>& chunkOffset) const {
    size_t headerAddress = read_u64(12 + dimensionSize() * chunkDims());
    H5BTreeVersion2 btree(fileAddress(), headerAddress);
    return fileAddress() + btree.getChunkAddressByOffset(chunkOffset);
}

H5Object H5DataLayoutMsg::extractDataChunk(
        const std::vector<size_t>& offset) const {
    const size_t keySize = 8 + offset.size() * 8;
    const size_t childSize = 8;
    uint64_t linkOffset = this->read_u64(3);
    H5BLinkNode bTree(fileAddress(), linkOffset);

    while (bTree.nodeLevel() > 0) {
        bool found = false;
        for (int i = bTree.entriesUsed() - 1; i >= 0; --i) {
            H5Object key(bTree + 24 + i * (keySize + childSize));
            if (chunkCompareGreaterEqual(offset.data(),
                                         (const uint64_t*)key.address(8),
                                         offset.size()))
            {
                bTree = H5BLinkNode(key.fileAddress(), key.read_u64(keySize));
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

bool H5DataLayoutMsg::isChunked() const {
    return _isChunked;
}

std::vector<size_t> H5DataLayoutMsg::chunkShape() const {
    return _chunkShape;
}
