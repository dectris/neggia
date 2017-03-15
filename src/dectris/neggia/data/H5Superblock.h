#ifndef H5SUPERBLOCK_H
#define H5SUPERBLOCK_H
#include "H5Object.h"
#include "H5SymbolTableEntry.h"

/// See https://www.hdfgroup.org/HDF5/doc/H5.format.html#Superblock

class H5Superblock : public H5Object
{
public:
   H5Superblock() = default;
   H5Superblock(const char * fileAddress);
   int groupLeafNodeK() const;
   int groupInternalNodeK() const;
   H5SymbolTableEntry rootGroupSymbolTableEntry() const;

};

#endif // H5SUPERBLOCK_H
