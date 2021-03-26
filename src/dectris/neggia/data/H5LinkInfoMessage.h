// SPDX-License-Identifier: MIT

#ifndef H5LINKINFOMESSAGE_H
#define H5LINKINFOMESSAGE_H
#include "H5LinkMsg.h"
#include "H5Object.h"

/// https://support.hdfgroup.org/HDF5/doc/H5.format.html#LinkInfoMessage

class H5LinkInfoMsg : public H5Object {
public:
    H5LinkInfoMsg() = default;
    H5LinkInfoMsg(const char* fileAddress, size_t offset);
    H5LinkInfoMsg(const H5Object& other);
    constexpr static unsigned int TYPE_ID = 0x02;
    uint8_t getFlags() const;
    bool existsMaximumCreationIndex() const;

    H5LinkMsg getLinkMessage(const char* rootFileAddress,
                             const std::string& pathItem) const;

private:
    uint64_t getFractalHeapAddress() const;
    uint64_t getBTreeAddress() const;
};

#endif  // H5LINKINFOMESSAGE_H
