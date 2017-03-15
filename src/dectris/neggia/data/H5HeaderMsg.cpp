#include "H5HeaderMsg.h"
#include <assert.h>

H5HeaderMsgPreamble::H5HeaderMsgPreamble(const char *fileAddress, size_t offset):
   H5Object(fileAddress, offset)
{
   this->_init();
}

H5HeaderMsgPreamble::H5HeaderMsgPreamble(const H5Object &other):
   H5Object(other)
{
    this->_init();
}

H5Object H5HeaderMsgPreamble::getHeaderMsg() const
{
    return at(DATA_OFFSET);
}

uint16_t H5HeaderMsgPreamble::type() const
{
   return uint16(0);
}

uint16_t H5HeaderMsgPreamble::size() const
{
   return uint16(2);
}

uint8_t H5HeaderMsgPreamble::flags() const
{
   assert(this->size() > 0 || this->type() == 0);
   return uint8(4);
}

void H5HeaderMsgPreamble::_init()
{
   assert(this->type() <= 0x18);
   assert(this->uint8(5) == 0);
   assert(this->uint8(6) == 0);
   assert(this->uint8(7) == 0);
   assert((flags() & 0x2) == 0); // We do not support shared messages
}

