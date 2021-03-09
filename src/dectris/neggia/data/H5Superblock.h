// SPDX-License-Identifier: MIT

#ifndef H5SUPERBLOCK_H
#define H5SUPERBLOCK_H
#include "H5Object.h"
#include "H5SymbolTableEntry.h"
#include "ResolvedPath.h"

/// See https://www.hdfgroup.org/HDF5/doc/H5.format.html#Superblock

class H5Superblock : public H5Object {
public:
    H5Superblock() = default;
    H5Superblock(const char* fileAddress);
    uint8_t version() const;

    ResolvedPath resolve(const H5Path& path);

private:
    ResolvedPath resolveV0(const H5Path& path);
    ResolvedPath resolveV2(const H5Path& path);
};

#endif  // H5SUPERBLOCK_H
