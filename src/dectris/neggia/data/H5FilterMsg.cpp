// SPDX-License-Identifier: MIT

#include "H5FilterMsg.h"
#include <assert.h>
#include <stdexcept>

H5FilterMsg::H5FilterMsg(const char* fileAddress, size_t offset)
      : H5Object(fileAddress, offset) {
    this->_init();
}

H5FilterMsg::H5FilterMsg(const H5Object& obj) : H5Object(obj) {
    this->_init();
}

uint8_t H5FilterMsg::version() const {
    return read_u8(0);
}

unsigned int H5FilterMsg::nFilters() const {
    return read_u8(1);
}

uint16_t H5FilterMsg::filterId(int i) const {
    return _filters.at(i).id;
}

std::string H5FilterMsg::filterName(int i) const {
    return _filters.at(i).name;
}

std::vector<int32_t> H5FilterMsg::clientData(int i) const {
    return _filters.at(i).client_data;
}

void H5FilterMsg::_init() {
    switch (version()) {
        case 1:
            _initV1();
            break;
        case 2:
            _initV2();
            break;
        default:
            throw std::runtime_error("Filter Pipeline Message version " +
                                     std::to_string(version()) +
                                     " not supported.");
    }
}

void H5FilterMsg::_initV1() {
    assert(read_u16(2) == 0);
    assert(read_u32(4) == 0);
    size_t currentOffset = 8;
    for (size_t fId = 0; fId < nFilters(); ++fId) {
        uint16_t filterId = read_u16(currentOffset);
        uint16_t nameLength = read_u16(currentOffset + 2);
        uint16_t nClientValues = read_u16(currentOffset + 6);
        std::string name;
        if (nameLength > 0) {
            name = std::string(address() + currentOffset + 8);
            // nameLength includes null-terminator and padding bytes
            assert(name.size() <= nameLength);
        }

        int32_t* array = (int32_t*)address(currentOffset + 8 + nameLength +
                                           (nameLength % 8));
        auto client_data = std::vector<int32_t>(array, array + nClientValues);
        _filters.push_back({filterId, name, client_data});
        currentOffset += 8 + nameLength + (nameLength % 8) + 4 * nClientValues +
                         (nClientValues % 2) * 4;
    }
}

void H5FilterMsg::_initV2() {
    size_t currentOffset = 2;
    for (size_t fId = 0; fId < nFilters(); ++fId) {
        uint16_t filterId = read_u16(currentOffset);
        std::string name;
        size_t flagsOffset = 0;
        size_t dataClientOffset = 0;
        if (filterId < 256) {
            flagsOffset = 2;
            dataClientOffset = 6;
            switch (filterId) {
                case 0:
                    name = "N/A";
                    break;
                case 1:
                    name = "deflate";
                    break;
                case 2:
                    name = "shuffle";
                    break;
                case 3:
                    name = "fletcher32";
                    break;
                case 4:
                    name = "szip";
                    break;
                case 5:
                    name = "nbit";
                    break;
                case 6:
                    name = "scaleoffset";
                    break;
                default:
                    name = "undefined";
            }
        } else {
            flagsOffset = 4;
            uint16_t nameLength = read_u16(currentOffset + 2);
            dataClientOffset = 8 + nameLength;
            if (nameLength > 0) {
                if (read_u8(currentOffset + 8 + nameLength) == 0) {
                    nameLength--;
                }
                name = std::string(address() + currentOffset + 8, nameLength);
            }
        }
        uint16_t flags = read_u16(currentOffset + flagsOffset);
        uint16_t nClientValues = read_u16(currentOffset + flagsOffset + 2);
        int32_t* array = (int32_t*)address(currentOffset + dataClientOffset);
        auto client_data = std::vector<int32_t>(array, array + nClientValues);
        _filters.push_back({filterId, name, client_data});
        currentOffset += dataClientOffset + 4 * nClientValues;
    }
}
