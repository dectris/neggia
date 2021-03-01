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

#ifndef H5DATATYPEMSG_H
#define H5DATATYPEMSG_H
#include "H5Object.h"
/// https://www.hdfgroup.org/HDF5/doc/H5.format.html#DatatypeMessage

class H5DatatypeMsg : public H5Object {
public:
    H5DatatypeMsg() = default;
    H5DatatypeMsg(const char* fileAddress, size_t offset);
    H5DatatypeMsg(const H5Object&);
    unsigned int version() const;
    unsigned int typeId() const;
    unsigned int dataSize() const;
    bool isSigned() const;
    constexpr static unsigned int TYPE_ID = 0x3;

private:
    void _init();
};

#endif  // H5DATATYPEMSG_H
