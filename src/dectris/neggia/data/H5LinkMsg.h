// SPDX-License-Identifier: MIT

#ifndef H5LINKMSG_H
#define H5LINKMSG_H
#include "H5Object.h"

/// https://www.hdfgroup.org/HDF5/doc/H5.format.html#LinkMessage

class H5LinkMsg : public H5Object {
public:
    enum LinkType { HARD, SOFT, EXTERNAL };

    H5LinkMsg() = default;
    H5LinkMsg(const char* fileAddress, size_t offset);
    H5LinkMsg(const H5Object& other);
    constexpr static unsigned int TYPE_ID = 0x06;
    std::string linkName() const;
    LinkType linkType() const;
    std::string targetFile() const;
    std::string targetPath() const;
    /// can be converted to H5ObjectHeader
    const H5Object& hardLinkObjectHeader() const;

private:
    void _init();
    std::string _linkName;
    LinkType _linkType;
    std::string _targetFile;
    std::string _targetPath;
    H5Object _hardLinkObjectHeader;
};

#endif  // H5LINKMSG_H
