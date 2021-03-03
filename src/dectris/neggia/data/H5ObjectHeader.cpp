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
#ifdef DEBUG_PARSING
#include "H5BLinkNode.h"
#include "H5DataLayoutMsg.h"
#include "H5DataspaceMsg.h"
#include "H5DatatypeMsg.h"
#include "H5FilterMsg.h"
#include "H5LinkInfoMessage.h"
#include "H5LinkMsg.h"
#endif

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

#ifdef DEBUG_PARSING
    std::cerr << " >> Parsing version 1 object header [number of messages: "
              << numberOfMessagesToParse << "]"
              << "\n";
#endif

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
#ifdef DEBUG_PARSING
        _printMsgDebug(currentMessage);
#endif
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

#ifdef DEBUG_PARSING
void H5ObjectHeader::_printMsgDebug(const H5HeaderMessage& msg) {
    switch (msg.type) {
        case 0x0:
            std::cerr << "  Nil Message [0x0]"
                      << "\n";
            break;
        case H5DataspaceMsg::TYPE_ID: {
            auto typedMsg = H5DataspaceMsg(msg.object);
            std::cerr << "Dataspace Message [0x" << std::hex
                      << H5DataspaceMsg::TYPE_ID << std::dec
                      << "] - version: " << (int)typedMsg.version()
                      << ", rank: " << (int)typedMsg.rank() << "\n";
            break;
        }
        case H5LinkInfoMsg::TYPE_ID: {
            auto typedMsg = H5LinkInfoMsg(msg.object);
            std::cerr << "Link Info Message [0x" << std::hex
                      << H5LinkInfoMsg::TYPE_ID
                      << "] - fractal heap address: 0x"
                      << typedMsg.getFractalHeapAddress()
                      << ", b-tree address: 0x" << typedMsg.getBTreeAddress()
                      << std::dec << "\n";
            break;
        }
        case H5DatatypeMsg::TYPE_ID: {
            auto typedMsg = H5DatatypeMsg(msg.object);
            std::cerr << "Datatype Message [0x" << std::hex
                      << H5DatatypeMsg::TYPE_ID << std::dec << "] - version: 0x"
                      << typedMsg.version() << ", type id: 0x"
                      << typedMsg.typeId()
                      << ", data size: " << (int)typedMsg.dataSize() << "\n";
            break;
        }
        case 0x4:
            std::cerr << "  Fill Value (Old) Message [0x4]"
                      << "\n";
            break;
        case 0x5:
            std::cerr << "  Fill Value Message [0x5]"
                      << "\n";
            break;
        case H5LinkMsg::TYPE_ID: {
            auto typedMsg = H5LinkMsg(msg.object);
            std::cerr << "Link Message [0x" << std::hex << H5LinkMsg::TYPE_ID
                      << std::dec << "] - link name: '" << typedMsg.linkName()
                      << "', target file: '" << typedMsg.targetFile()
                      << "', target path: '" << typedMsg.targetPath() << "'\n";
            break;
        }
        case H5DataLayoutMsg::TYPE_ID: {
            auto typedMsg = H5DataLayoutMsg(msg.object);
            std::cerr << "DataLayout Message [0x" << std::hex
                      << H5DataLayoutMsg::TYPE_ID << std::dec
                      << "] - layout class: " << (int)typedMsg.layoutClass()
                      << "\n";
            break;
        }
        case 0xa:
            std::cerr << "  Group Info Message [0xa]"
                      << "\n";
            break;
        case H5FilterMsg::TYPE_ID: {
            auto typedMsg = H5FilterMsg(msg.object);
            std::cerr << "Filter Message [0x" << std::hex
                      << H5FilterMsg::TYPE_ID << "] - version: " << std::dec
                      << (int)typedMsg.version() << ", filters: [";
            if (typedMsg.nFilters() > 0) {
                for (int i = 0; i < (int)typedMsg.nFilters(); ++i) {
                    std::cerr << "'" << typedMsg.filterName(i)
                              << " {id:" << (int)typedMsg.filterId(i) << "}', ";
                }
            }
            std::cerr << "] (size: " << (int)typedMsg.nFilters() << ")\n";
            break;
        }
        case 0xc:
            std::cerr << "  Attribute Message [0xc]"
                      << "\n";
            break;
        case 0x10:
            std::cerr << "  Object Header Continuation Message [0x10]"
                      << "\n";
            break;
        case 0x15:
            std::cerr << "  Attribute Info Message [0x15]"
                      << "\n";
            break;
        default:
            std::cerr << "  Unknown Message: 0x" << std::hex << msg.type
                      << std::dec << "\n";
    }
}
#endif