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

#ifndef H5OBJECTHEADER_H
#define H5OBJECTHEADER_H
#include <vector>
#include "H5HeaderMsg.h"

/// https://support.hdfgroup.org/HDF5/doc/H5.format.html#ObjectHeaderPrefix

class H5ObjectHeader : public H5Object {
public:
    H5ObjectHeader() = default;
    H5ObjectHeader(const char* fileAddress, size_t offset);
    H5ObjectHeader(const H5Object& other);
    uint16_t numberOfMessages() const;
    H5HeaderMessage headerMessage(int i) const;

private:
    std::vector<H5HeaderMessage> _messages;
    int version() const;

    void _init();
    void _initV1();
};

#endif  // H5OBJECTHEADER_H
