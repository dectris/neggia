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
   return this->uint8(1);
}

size_t H5DataLayoutMsg::dataSize() const {
   switch(layoutClass()) {
   case 0:
      return uint16(2);
   case 1:
      return uint64(2 + 8);
   default:
      throw std::runtime_error("wrong layout class");
   }
}

const char * H5DataLayoutMsg::dataAddress() const {
   switch(layoutClass()) {
   case 0:
      return address() + 2 + 2;
   case 1:
      return fileAddress() + uint64(2);
   default:
      throw std::runtime_error("wrong layout class");
   }
}

uint8_t H5DataLayoutMsg::chunkDims() const
{
   assert(layoutClass() == 2);
   return this->uint8(DEBUG_OFFSET + 2);
}

H5BLinkNode H5DataLayoutMsg::chunkBTree() const
{
   assert(layoutClass() == 2);
   uint64_t linkOffset = this->uint64(DEBUG_OFFSET + 3);
   return H5BLinkNode(fileAddress(),linkOffset);
}

uint32_t H5DataLayoutMsg::chunkDim(int i) const
{
   assert(layoutClass() == 2);
   return this->uint32(DEBUG_OFFSET + 11 + 4*i);
}

void H5DataLayoutMsg::_init()
{
   assert(this->uint8(0) == 3); //Version == 3
   assert(layoutClass() < 3);
}
