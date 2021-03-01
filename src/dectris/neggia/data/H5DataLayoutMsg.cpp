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

#include "H5DataLayoutMsg.h"
#include <assert.h>
#include <iostream>
#include <stdexcept>

#define DEBUG_OFFSET 0


H5DataLayoutMsg::H5DataLayoutMsg(const char *fileAddress, size_t offset):
   H5Object(fileAddress,offset)
{
    this->_init();
}

H5DataLayoutMsg::H5DataLayoutMsg(const H5Object & obj): H5Object(obj)
{
    this->_init();
}


uint8_t H5DataLayoutMsg::layoutClass() const
{
   return this->read_u8(1);
}

size_t H5DataLayoutMsg::dataSize() const {
   switch(layoutClass()) {
   case 0:
      return read_u16(2);
   case 1:
      return read_u64(2 + 8);
   default:
      throw std::runtime_error("wrong layout class");
   }
}

const char * H5DataLayoutMsg::dataAddress() const {
   switch(layoutClass()) {
   case 0:
      return address() + 2 + 2;
   case 1:
      return fileAddress() + read_u64(2);
   default:
      throw std::runtime_error("wrong layout class");
   }
}

uint8_t H5DataLayoutMsg::chunkDims() const
{
   assert(layoutClass() == 2);
   return this->read_u8(DEBUG_OFFSET + 2);
}

H5BLinkNode H5DataLayoutMsg::chunkBTree() const
{
   assert(layoutClass() == 2);
   uint64_t linkOffset = this->read_u64(DEBUG_OFFSET + 3);
   return H5BLinkNode(fileAddress(),linkOffset);
}

uint32_t H5DataLayoutMsg::chunkDim(int i) const
{
   assert(layoutClass() == 2);
   return this->read_u32(DEBUG_OFFSET + 11 + 4*i);
}

void H5DataLayoutMsg::_init()
{
   assert(this->read_u8(0) == 3); //Version == 3
   assert(layoutClass() < 3);
}
