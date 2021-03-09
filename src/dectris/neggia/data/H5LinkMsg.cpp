// SPDX-License-Identifier: MIT

#include "H5LinkMsg.h"
#include <assert.h>
#include <stdexcept>

H5LinkMsg::H5LinkMsg(const char* fileAddress, size_t offset)
      : H5Object(fileAddress, offset) {
    this->_init();
}

H5LinkMsg::H5LinkMsg(const H5Object& other) : H5Object(other) {
    this->_init();
}

std::string H5LinkMsg::linkName() const {
    return _linkName;
}

H5LinkMsg::LinkType H5LinkMsg::linkType() const {
    return _linkType;
}

std::string H5LinkMsg::targetFile() const {
    return _targetFile;
}

std::string H5LinkMsg::targetPath() const {
    return _targetPath;
}

const H5Object& H5LinkMsg::hardLinkObjectHeader() const {
    return _hardLinkObjectHeader;
}

void H5LinkMsg::_init() {
    assert(read_u8(0) == 1);  // version == 1
    uint8_t flags = read_u8(1);
    bool creationOrderIsPresent = flags & 0x4;
    bool linkTypeFieldIsPresent = flags & 0x8;
    bool linkNameCharacterSetFieldIsPresent = flags & 0x10;

    if (linkTypeFieldIsPresent) {
        uint8_t lt = read_u8(2);
        switch (lt) {
            case 0:
                _linkType = HARD;
                break;
            case 1:
                _linkType = SOFT;
                break;
            case 64:
                _linkType = EXTERNAL;
                break;
            default:
                assert(false);
        }
    } else {
        _linkType = HARD;
    }

    uint8_t lengthOfLinkNameOffset = 2 + linkTypeFieldIsPresent +
                                     creationOrderIsPresent * 8 +
                                     linkNameCharacterSetFieldIsPresent;

    uint8_t lengthOfLinkNameSize = 0;
    size_t lengthOfLinkName = 0;
    switch (flags & 0x3) {
        case 0:
            lengthOfLinkNameSize = 1;
            lengthOfLinkName = read_u8(lengthOfLinkNameOffset);
            break;
        case 1:
            lengthOfLinkName = read_u16(lengthOfLinkNameOffset);
            lengthOfLinkNameSize = 2;
            break;
        case 2:
            lengthOfLinkName = read_u32(lengthOfLinkNameOffset);
            lengthOfLinkNameSize = 4;
            break;
        case 3:
            lengthOfLinkName = read_u64(lengthOfLinkNameOffset);
            lengthOfLinkNameSize = 8;
            break;
        default:
            assert(false);
    }
    size_t linkNameOffset = lengthOfLinkNameOffset + lengthOfLinkNameSize;
    _linkName = std::string(address(linkNameOffset), lengthOfLinkName);
    size_t linkInformationOffset = linkNameOffset + lengthOfLinkName;

    switch (_linkType) {
        case HARD:
            _hardLinkObjectHeader =
                    H5Object(fileAddress(), read_u64(linkInformationOffset));
            break;
        case SOFT: {
            size_t length = read_u16(linkInformationOffset);
            _targetFile = "";
            _targetPath =
                    std::string(address(linkInformationOffset + 2), length);
            break;
        }
        case EXTERNAL: {
            size_t length = read_u16(linkInformationOffset) -
                            1;  // -1: first byte is used for version number
            linkInformationOffset += 3;
            assert(read_u8(linkInformationOffset + length - 1) ==
                   0);  // assert second string is null terminated
            for (size_t i = 0; i < length - 1; ++i) {
                if (read_u8(linkInformationOffset + i) == 0) {
                    _targetFile =
                            std::string(address(linkInformationOffset), i);
                    _targetPath =
                            std::string(address(linkInformationOffset + i + 1),
                                        length - i - 2);
                    break;
                }
            }
            break;
        }
        default:
            assert(false);
    }
}
