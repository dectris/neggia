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

#include "H5ObjectHeader.h"
#include <assert.h>
#include <iostream>
#include <stack>

H5ObjectHeader::H5ObjectHeader(const char* fileAddress, size_t offset)
      : H5Object(fileAddress, offset) {
    _init();
}

H5ObjectHeader::H5ObjectHeader(const H5Object& other) : H5Object(other) {
    _init();
}

uint16_t H5ObjectHeader::numberOfMessages() const {
    return read_u16(2);
}

uint32_t H5ObjectHeader::referenceCount() const {
    return read_u32(4);
}

uint32_t H5ObjectHeader::headerSize() const {
    return read_u32(8);
}

uint16_t H5ObjectHeader::messageType(int i) const {
    return *(const uint16_t*)(fileAddress() + _messageOffset[i]);
}

uint16_t H5ObjectHeader::messageSize(int i) const {
    return *(const uint16_t*)(fileAddress() + _messageOffset[i] + 2);
}

uint8_t H5ObjectHeader::messageFlags(int i) const {
    return *(const uint8_t*)(fileAddress() + _messageOffset[i] + 4);
}

H5HeaderMessage H5ObjectHeader::headerMessage(int i) const {
    return H5HeaderMessage{H5Object(fileAddress(), _messageOffset[i] + 8),
                           messageType(i)};
}

void H5ObjectHeader::_init() {
    assert(read_u8(0) == 1);
    assert(read_u8(1) == 0);

    constexpr uint64_t INVALID_SIZE = 0xffffffffffffffff;

    size_t messageOffset = offset() + 16;  // 12(header data) + 4(alignment)

    struct ContBlock {
        uint64_t addr;
        uint64_t size;
    };
    std::stack<ContBlock> continuationBlocks;

    uint64_t currentBlockSize = headerSize();
    uint64_t currentMessageSize = 0;
    int messageId = 0;
    while (true) {
        _messageOffset.push_back(messageOffset);
        H5Object messageObject(fileAddress(), messageOffset);
        uint16_t messageSize_ = messageObject.read_u16(2);
        assert(messageSize_ == messageSize(messageId));
        assert(messageSize_ % 8 == 0);
        assert(messageObject.read_u8(5) == 0);
        assert(messageObject.read_u8(6) == 0);
        assert(messageObject.read_u8(7) == 0);
        if (messageType(messageId) == 0x10) {
            auto contMsg = headerMessage(messageId).object;
            uint64_t cbl = contMsg.read_u64(0);
            if (cbl != INVALID_SIZE) {
                continuationBlocks.push({cbl, contMsg.read_u64(8)});
            }
        }
        assert(messageObject.read_u16(0) == messageType(messageId));
        assert(messageObject.read_u16(0) <= 0x18);
        if (++messageId < numberOfMessages()) {
            uint16_t size = messageSize_ + 8;
            currentMessageSize += size;
            assert(currentMessageSize <= currentBlockSize);
            if (currentMessageSize == currentBlockSize) {
                currentMessageSize = 0;
                assert(!continuationBlocks.empty());
                messageOffset = continuationBlocks.top().addr;
                currentBlockSize = continuationBlocks.top().size;
                continuationBlocks.pop();
            } else {
                messageOffset += size;
            }
        } else {
            break;
        }
    }
}
