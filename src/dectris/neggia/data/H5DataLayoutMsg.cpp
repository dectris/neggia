// SPDX-License-Identifier: MIT

#include "H5DataLayoutMsg.h"
#include <assert.h>
#include <string.h>
#include <iostream>
#include <stdexcept>

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
    return this->read_u8(DEBUG_OFFSET + 2);
}

H5BLinkNode H5DataLayoutMsg::chunkBTree() const {
    assert(layoutClass() == 2);
    uint64_t linkOffset = this->read_u64(DEBUG_OFFSET + 3);
    return H5BLinkNode(fileAddress(), linkOffset);
}

uint32_t H5DataLayoutMsg::chunkDim(int i) const {
    assert(layoutClass() == 2);
    return this->read_u32(DEBUG_OFFSET + 11 + 4 * i);
}

void H5DataLayoutMsg::_init() {
    assert(version() == 3);
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
        const std::vector<size_t>& chunkOffset) const {
    const char* rawData = nullptr;
    size_t rawDataSize = 0;
    if (_isChunked) {
        // internally hdf5 stores chunk size with one dimension more than the
        // dimensions of the dataset
        // https://www.hdfgroup.org/HDF5/doc/H5.format.html#LayoutMessage
        std::vector<size_t> chunkOffsetFullSize(chunkOffset);
        while (chunkOffsetFullSize.size() < _chunkShape.size() + 1)
            chunkOffsetFullSize.push_back(0);
        H5Object dataChunk(extractDataChunk(chunkOffsetFullSize));
        rawData = dataChunk.fileAddress() +
                  dataChunk.read_u64(8 + chunkOffsetFullSize.size() * 8);
        rawDataSize = dataChunk.read_u32(0);
    } else {
        rawData = dataAddress();
        rawDataSize = dataSize();
    }
    return ConstDataPointer{rawData, rawDataSize};
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

H5Object H5DataLayoutMsg::extractDataChunk(
        const std::vector<size_t>& offset) const {
    const size_t keySize = 8 + offset.size() * 8;
    const size_t childSize = 8;
    H5BLinkNode bTree(chunkBTree());

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
