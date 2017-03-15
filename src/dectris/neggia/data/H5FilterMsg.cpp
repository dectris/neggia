#include "H5FilterMsg.h"
#include <assert.h>


H5FilterMsg::H5FilterMsg(const char *fileAddress, size_t offset):
   H5Object(fileAddress,offset)
{
    this->_init();
}

H5FilterMsg::H5FilterMsg(const H5Object & obj): H5Object(obj)
{
    this->_init();
}

unsigned int H5FilterMsg::nFilters() const
{
   return uint8(1);
}

uint16_t H5FilterMsg::filterId(int i) const
{
   return uint16(_offset[i]);
}

std::string H5FilterMsg::filterName(int i) const
{
   return std::string(address(_offset[i]+8));
}


std::vector<int32_t> H5FilterMsg::clientData(int i) const
{
   unsigned int nameLength = uint16(_offset[i]+2);
   uint16_t nClientDataValues = uint16(_offset[i]+6);
   int32_t * array = (int32_t*) address(_offset[i]+8+nameLength);
   return std::vector<int32_t>(array, array + nClientDataValues);
}



void H5FilterMsg::_init()
{
   assert(uint8(0) == 1); //VERSION 1
   assert(uint16(2) == 0);
   assert(uint32(4) == 0);
   size_t filterOffset = 8;
   for(size_t fId=0; fId<nFilters(); ++fId) {
      _offset.push_back(filterOffset);
      unsigned int nLength = uint16(filterOffset+2);
      assert(nLength%8==0);
      unsigned int nClientValues = uint16(filterOffset+6);
      filterOffset += nLength + 4*nClientValues;
      if(nClientValues%2) filterOffset += 4;
   }
}
