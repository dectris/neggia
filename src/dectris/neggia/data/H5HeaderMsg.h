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
