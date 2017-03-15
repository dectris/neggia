#include "H5Superblock.h"
#include "H5SymbolTableEntry.h"
#include "constants.h"
#include <assert.h>



H5Superblock::H5Superblock(const char *fileAddress):
   H5Object(fileAddress,0)
{
   const char magicNumber[] = "\211HDF\r\n\032\n";
   assert(std::string(fileAddress,8) == std::string(magicNumber));
   int version = (int)fileAddress[8];
   assert(version == 0);
   int offsetSize = (int)fileAddress[13];
   assert(offsetSize == 8);
   int offsetLength = (int)fileAddress[14];
   assert(offsetLength == 8);
   assert(fileAddress[15] == 0);
   uint64_t baseAddress = *(uint64_t*)(fileAddress+24);
   assert(baseAddress == 0);
   uint64_t DriverInformationBlockAddress = *(uint64_t*)(fileAddress+24+3*offsetSize);
   assert (DriverInformationBlockAddress == H5_INVALID_ADDRESS);
}

int H5Superblock::groupLeafNodeK() const
{
   return  uint16(16); //*(uint16_t*)(fileAddress(16));
}

int H5Superblock::groupInternalNodeK() const
{
   return  uint16(18); //return  *(uint16_t*)(fileAddress(18));
}

H5SymbolTableEntry H5Superblock::rootGroupSymbolTableEntry() const
{
   return H5SymbolTableEntry(at(24+4*8)); //(const char *)fileAddress(24+4*8);
}

