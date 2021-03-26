// SPDX-License-Identifier: MIT

#include "H5ObjectHeader.h"
#include <assert.h>
#include <iostream>
#include <stack>
#include <stdexcept>
#ifdef DEBUG_PARSING
#include <bitset>
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
        case 2:
            _initV2();
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

void H5ObjectHeader::_initV2() {
    uint8_t flags = read_u8(5);
    size_t skipOptionalBytes = 0;
    if (flags & (1 << 4)) {
        skipOptionalBytes += 4;
    }
    if (flags & (1 << 5)) {
        skipOptionalBytes += 16;
    }
    size_t optionalBytesInMessageHeader = 0;
    if (flags & (1 << 2)) {
        optionalBytesInMessageHeader = 2;
    }
    size_t chunk0Size, currentOffset;
    switch (flags & 3) {
        case 0:
            chunk0Size = read_u8(6 + skipOptionalBytes);
            currentOffset = 6 + skipOptionalBytes + 1;
            break;
        case 1:
            chunk0Size = read_u16(6 + skipOptionalBytes);
            currentOffset = 6 + skipOptionalBytes + 2;
            break;
        case 2:
            chunk0Size = read_u32(6 + skipOptionalBytes);
            currentOffset = 6 + skipOptionalBytes + 4;
            break;
        case 3:
            chunk0Size = read_u64(6 + skipOptionalBytes);
            currentOffset = 6 + skipOptionalBytes + 8;
            break;
    }
#ifdef DEBUG_PARSING
    std::cerr << " >> Parsing version 2 object header [chunk0size: "
              << chunk0Size << ", flags: 0b" << std::bitset<8>(flags) << "]"
              << std::dec << std::endl;
#endif
    _addBlockV2(currentOffset, chunk0Size, optionalBytesInMessageHeader);
}

void H5ObjectHeader::_addBlockV2(size_t currentOffset,
                                 size_t blockSize,
                                 size_t optionalBytesInMessageHeader) {
    constexpr uint64_t INVALID_OFFSET = 0xffffffffffffffff;
    struct ContBlock {
        uint64_t addr;
        uint64_t size;
    };
    std::stack<ContBlock> continuationBlocks;

    size_t end_headers = currentOffset + blockSize;
    while (currentOffset < end_headers) {
        uint16_t messageType = read_u8(currentOffset);
        uint16_t messageSize = read_u16(currentOffset + 1);
        uint8_t messageFlags = read_u8(currentOffset + 3);
        currentOffset += 4 + optionalBytesInMessageHeader;
        auto currentMessage = H5HeaderMessage{
                H5Object(fileAddress(), offset() + currentOffset), messageType};
        _messages.push_back(currentMessage);
#ifdef DEBUG_PARSING
        _printMsgDebug(currentMessage);
#endif

        currentOffset += messageSize;
        if (currentMessage.type == 0x10) {
            uint64_t addr = currentMessage.object.read_u64(0);
            uint64_t size = currentMessage.object.read_u64(8);
            if (addr != INVALID_OFFSET) {
                continuationBlocks.push({addr, size});
            }
        }
    }
    while (!continuationBlocks.empty()) {
#ifdef DEBUG_PARSING
        std::cerr << " >> Add version 2 continuation block" << std::endl;
#endif
        const char signatureContinuationBlockV2[] = "OCHK";
        assert(std::string(&fileAddress()[continuationBlocks.top().addr], 4) ==
               std::string(signatureContinuationBlockV2));
        size_t checkSumSize = 8;
        size_t signatureSize = 4;
        size_t contOffset = continuationBlocks.top().addr + 4 - offset();
        _addBlockV2(contOffset, continuationBlocks.top().size - checkSumSize,
                    optionalBytesInMessageHeader);
        continuationBlocks.pop();
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
            std::cerr << "  Dataspace Message [0x" << std::hex
                      << H5DataspaceMsg::TYPE_ID << std::dec
                      << "] - version: " << (int)typedMsg.version()
                      << ", rank: " << (int)typedMsg.rank() << "\n";
            break;
        }
        case H5LinkInfoMsg::TYPE_ID: {
            auto typedMsg = H5LinkInfoMsg(msg.object);
            std::cerr << "  Link Info Message [0x" << std::hex
                      << H5LinkInfoMsg::TYPE_ID << "]" << std::dec << "\n";
            break;
        }
        case H5DatatypeMsg::TYPE_ID: {
            auto typedMsg = H5DatatypeMsg(msg.object);
            std::cerr << "  Datatype Message [0x" << std::hex
                      << H5DatatypeMsg::TYPE_ID << std::dec
                      << "] - version: " << (int)typedMsg.version()
                      << ", type id: " << typedMsg.typeId()
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
            std::cerr << "  Link Message [0x" << std::hex << H5LinkMsg::TYPE_ID
                      << std::dec << "] - link name: '" << typedMsg.linkName()
                      << "', link type: " << (int)typedMsg.linkType()
                      << ", target file: '" << typedMsg.targetFile()
                      << "', target path: '" << typedMsg.targetPath() << "'\n";
            break;
        }
        case H5DataLayoutMsg::TYPE_ID: {
            auto typedMsg = H5DataLayoutMsg(msg.object);
            std::cerr << "  DataLayout Message [0x" << std::hex
                      << H5DataLayoutMsg::TYPE_ID << std::dec
                      << "] - version: " << (int)typedMsg.version()
                      << ", layout class: " << (int)typedMsg.layoutClass()
                      << "\n";
            break;
        }
        case 0xa:
            std::cerr << "  Group Info Message [0xa]"
                      << "\n";
            break;
        case H5FilterMsg::TYPE_ID: {
            auto typedMsg = H5FilterMsg(msg.object);
            std::cerr << "  Filter Message [0x" << std::hex
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
