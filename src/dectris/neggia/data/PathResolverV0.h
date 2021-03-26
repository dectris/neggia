// SPDX-License-Identifier: MIT

#ifndef PATH_RESOLVER_V0_H
#define PATH_RESOLVER_V0_H
#include "H5LinkInfoMessage.h"
#include "H5LinkMsg.h"
#include "H5Path.h"
#include "H5SymbolTableEntry.h"
#include "ResolvedPath.h"

class PathResolverV0 {
public:
    PathResolverV0(const H5SymbolTableEntry& root);
    ResolvedPath resolve(const H5Path& path);

private:
    const H5SymbolTableEntry _root;

    ResolvedPath findPathInObjectHeader(const H5SymbolTableEntry& parentEntry,
                                        const H5ObjectHeader& objectHeader,
                                        const std::string pathItem,
                                        const H5Path& remainingPath);
    ResolvedPath findPathInLinkMsg(const H5SymbolTableEntry& parentEntry,
                                   const H5LinkMsg& linkMsg,
                                   const H5Path& remainingPath);
    ResolvedPath findPathInScratchSpace(H5SymbolTableEntry parentEntry,
                                        H5SymbolTableEntry symbolTableEntry,
                                        const H5Path& remainingPath);
    ResolvedPath resolvePathInSymbolTableEntry(const H5SymbolTableEntry& in,
                                               const H5Path& path);
};

#endif  // PATH_RESOLVER_V1_H
