#ifndef H5OBJECTHEADER_H
#define H5OBJECTHEADER_H
#include "H5HeaderMsg.h"
#include <vector>

/// https://www.hdfgroup.org/HDF5/doc/H5.format.html#ObjectHeaderPrefix

class H5ObjectHeader : public H5Object
{
public:
   H5ObjectHeader() = default;
   H5ObjectHeader(const char * fileAddress, size_t offset);
   H5ObjectHeader(const H5Object & other);
   uint16_t numberOfMessages() const;
   uint32_t referenceCount() const;
   uint32_t headerSize() const;
   uint16_t messageType(int i) const;
   uint16_t messageSize(int i) const;
   uint8_t messageFlags(int i) const;
   H5HeaderMsgPreamble messageData(int i) const;
private:
   std::vector<uint64_t> _messageOffset;
   void _init();
};

#endif // H5OBJECTHEADER_H
