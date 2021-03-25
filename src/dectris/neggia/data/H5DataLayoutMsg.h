// SPDX-License-Identifier: MIT

#ifndef H5DATALAYOUTMSG_H
#define H5DATALAYOUTMSG_H
#include "H5BLinkNode.h"
#include "H5Object.h"
#include "H5ObjectHeader.h"

/// https://www.hdfgroup.org/HDF5/doc/H5.format.html#LayoutMessage

class H5DataLayoutMsg : public H5Object {
public:
    struct ConstDataPointer {
        const char* data;
        size_t size;
    };
    H5DataLayoutMsg() = default;
    H5DataLayoutMsg(const char* fileAddress, size_t offset);
    H5DataLayoutMsg(const H5Object&);
    uint8_t version() const;
    uint8_t layoutClass() const;

    ConstDataPointer getRawData(size_t elementSize,
                                const std::vector<size_t>& chunkOffset) const;

    bool isChunked() const;
    std::vector<size_t> chunkShape() const;

    constexpr static unsigned int TYPE_ID = 0x8;

private:
    void _init();

    uint8_t chunkDims() const;
    uint32_t chunkDim(int i) const;

    /// for raw and contigous data  (layout class 0,1)
    size_t dataSize() const;
    const char* dataAddress() const;

    /// for chunked data (layout class 2)
    ConstDataPointer chunkedDataV3(
            const std::vector<size_t>& chunkOffset) const;
    ConstDataPointer chunkedDataV4(
            size_t elementSize,
            const std::vector<size_t>& chunkOffset) const;
    const char* addressFromFixedArrayStorage(
            const std::vector<size_t>& chunkOffset) const;
    const char* addressFromExtensibleArrayStorage(
            const std::vector<size_t>& chunkOffset) const;
    const char* addressFromBTreeV2Storage(
            const std::vector<size_t>& chunkOffset) const;

    H5Object extractDataChunk(const std::vector<size_t>& chunkOffset) const;
    size_t dimensionSize() const;
    uint8_t chunkIndexingType() const;

    bool _isChunked;
    std::vector<size_t> _chunkShape;
};

#endif  // H5DATALAYOUTMSG_H
