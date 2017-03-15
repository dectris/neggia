#ifndef H5FILTERMSG_H
#define H5FILTERMSG_H
#include <string>
#include <vector>
#include "H5Object.h"
/// https://www.hdfgroup.org/HDF5/doc/H5.format.html#FilterMessage

class H5FilterMsg: public H5Object
{
public:
   H5FilterMsg() = default;
   H5FilterMsg(const char * fileAddress, size_t offset);
   H5FilterMsg(const H5Object&);
   unsigned int nFilters() const;
   uint16_t filterId(int i) const;
   std::string filterName(int i) const;
   std::vector<int32_t> clientData(int i) const;
   constexpr static unsigned int TYPE_ID = 0xb;


private:
   void _init();
   std::vector<uint64_t> _offset;

};

#endif // H5FILTERMSG_H
