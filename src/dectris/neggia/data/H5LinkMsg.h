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
