// SPDX-License-Identifier: MIT

#include "H5LinkInfoMessage.h"
#include <stdexcept>
#include "H5BTreeVersion2.h"
#include "H5FractalHeap.h"
#include "constants.h"

H5LinkInfoMsg::H5LinkInfoMsg(const char* fileAddress, size_t offset)
      : H5Object(fileAddress, offset) {}

H5LinkInfoMsg::H5LinkInfoMsg(const H5Object& other) : H5Object(other) {}

uint8_t H5LinkInfoMsg::getFlags() const {
    return read_u8(1);
}

bool H5LinkInfoMsg::existsMaximumCreationIndex() const {
    return (bool)(getFlags() & 1);
}

uint64_t H5LinkInfoMsg::getFractalHeapAddress() const {
    if (existsMaximumCreationIndex())
        return read_u64(10);
    else
        return read_u64(2);
}

uint64_t H5LinkInfoMsg::getBTreeAddress() const {
    if (existsMaximumCreationIndex())
        return read_u64(18);
    else
        return read_u64(10);
}

H5LinkMsg H5LinkInfoMsg::getLinkMessage(const char* rootFileAddress,
                                        const std::string& pathItem) const {
    size_t btreeAddress = getBTreeAddress();
    if (btreeAddress == H5_INVALID_ADDRESS) {
        throw std::out_of_range("Invalid address");
    }
    H5BTreeVersion2 btree(rootFileAddress, btreeAddress);
    H5Object heapRecord(rootFileAddress, btree.getLinkAddressByName(pathItem));
    uint32_t heapOffset = heapRecord.read_u32(5);

    H5FractalHeap fractalHeap(rootFileAddress, getFractalHeapAddress());
    H5LinkMsg linkMsg(fractalHeap.getHeapObject(heapOffset));
    if (linkMsg.linkName() != pathItem) {
        throw std::out_of_range("item not found");
    }
    return linkMsg;
}
