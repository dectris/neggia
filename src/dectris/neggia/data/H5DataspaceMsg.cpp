#include "H5DataspaceMsg.h"
#include "assert.h"

H5DataspaceMsg::H5DataspaceMsg(const H5Object & obj): H5Object(obj)
{
    this->_init();
}

H5DataspaceMsg::H5DataspaceMsg(const char *fileAddress, size_t offset):
   H5Object(fileAddress,offset)
{
   this->_init();
}

uint8_t H5DataspaceMsg::rank() const
{
   return this->uint8(1);
}

bool H5DataspaceMsg::maxDims() const
{
   return this->uint8(2) & 0x1;
}

uint64_t H5DataspaceMsg::dim(int i) const
{
   return this->uint64(8 + i*8);
}

uint64_t H5DataspaceMsg::maxDim(int i) const
{
   assert(this->maxDims());
   return this->uint64(8 + this->rank()*8 + i*8);
}

void H5DataspaceMsg::_init()
{
   assert(this->uint8(0) == 1);
}

