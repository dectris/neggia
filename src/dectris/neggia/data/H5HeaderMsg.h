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

#ifndef H5HEADERMSG_H
#define H5HEADERMSG_H
#include "H5Object.h"

/// https://www.hdfgroup.org/HDF5/doc/H5.format.html#ObjectHeaderMessages
/// Contains type, size and flags of Header Message part of H5ObjectHeader:
/// https://www.hdfgroup.org/HDF5/doc/H5.format.html#ObjectHeaderPrefix

class H5HeaderMsgPreamble: public H5Object
{
public:
   H5HeaderMsgPreamble() = default;
   H5HeaderMsgPreamble(const char * fileAddress, size_t offset);
   H5HeaderMsgPreamble(const H5Object & other);
   H5Object getHeaderMsg() const;
   uint16_t type() const;
   uint16_t size() const;
   uint8_t flags() const;
   static constexpr int DATA_OFFSET = 8;

private:
   void _init();

};

#endif // H5HEADERMSG_H
