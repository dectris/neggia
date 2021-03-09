// SPDX-License-Identifier: MIT

#ifndef H5OBJECTHEADER_H
#define H5OBJECTHEADER_H
#include <vector>
#include "H5HeaderMsg.h"

/// https://support.hdfgroup.org/HDF5/doc/H5.format.html#ObjectHeaderPrefix

class H5ObjectHeader : public H5Object {
public:
    H5ObjectHeader() = default;
    H5ObjectHeader(const char* fileAddress, size_t offset);
    H5ObjectHeader(const H5Object& other);
    int version() const;
    uint16_t numberOfMessages() const;
    H5HeaderMessage headerMessage(int i) const;

private:
    std::vector<H5HeaderMessage> _messages;

    void _init();
    void _initV1();
    void _initV2();

    void _addBlockV2(size_t currentOffset,
                     size_t blockSize,
                     size_t optionalBytesInMessageHeader);
#ifdef DEBUG_PARSING
    void _printMsgDebug(const H5HeaderMessage& msg);
#endif
};

#endif  // H5OBJECTHEADER_H
