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
    return _messages.size();
}

H5HeaderMessage H5ObjectHeader::headerMessage(int i) const {
    return _messages.at(i);
}

int H5ObjectHeader::version() const {
    if (read_u8(0) == 1 && read_u8(1) == 0) {
        return 1;
    }
    if (std::string(address(), 4) == "OHDR" && read_u8(4) == 2) {
        return 2;
    }
    throw std::runtime_error("could not determine version of H5ObjectHeader");
}

void H5ObjectHeader::_init() {
    switch (version()) {
        case 1:
            _initV1();
            break;
        default:
            throw std::runtime_error("Object Header version " +
                                     std::to_string(version()) +
                                     " not supported.");
    }
}

void H5ObjectHeader::_initV1() {
    constexpr uint64_t INVALID_SIZE = 0xffffffffffffffff;

    size_t messageOffset = offset() + 16;  // 12(header data) + 4(alignment)

    struct ContBlock {
        uint64_t addr;
        uint64_t size;
    };
    std::stack<ContBlock> continuationBlocks;

    uint16_t numberOfMessagesToParse = read_u16(2);
    uint64_t currentBlockSize = read_u32(8);
    uint64_t currentMessageSize = 0;

    int messageId = 0;
    while (true) {
        auto currentMessage =
                H5HeaderMessage{H5Object(fileAddress(), messageOffset + 8),
                                read_u16(messageOffset - offset())};
        H5Object messageObject(fileAddress(), messageOffset);
        uint16_t messageSize = messageObject.read_u16(2);
        assert(messageSize == read_u16(messageOffset - offset() + 2));
        assert(messageSize % 8 == 0);
        assert(messageObject.read_u8(5) == 0);
        assert(messageObject.read_u8(6) == 0);
        assert(messageObject.read_u8(7) == 0);

        if (currentMessage.type == 0x10) {
            auto contMsg = currentMessage.object;
            uint64_t cbl = contMsg.read_u64(0);
            if (cbl != INVALID_SIZE) {
                continuationBlocks.push({cbl, contMsg.read_u64(8)});
            }
        }
        assert(messageObject.read_u16(0) == currentMessage.type);
        assert(messageObject.read_u16(0) <= 0x18);
        _messages.push_back(currentMessage);
        if (++messageId < numberOfMessagesToParse) {
            uint16_t size = messageSize + 8;
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
