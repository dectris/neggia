// SPDX-License-Identifier: MIT

#ifndef PATH_RESOLVER_V2_H
#define PATH_RESOLVER_V2_H
#include "H5LinkInfoMessage.h"
#include "H5LinkMsg.h"
#include "H5ObjectHeader.h"
#include "H5Path.h"
#include "ResolvedPath.h"

class PathResolverV2 {
public:
    PathResolverV2(const H5ObjectHeader& root);
    ResolvedPath resolve(const H5Path& path);

private:
    const H5ObjectHeader _root;

    ResolvedPath resolvePathInHeader(const H5ObjectHeader& in,
                                     const H5Path& path);
    ResolvedPath findPathInObjectHeader(const H5ObjectHeader& parentEntry,
                                        const std::string pathItem,
                                        const H5Path& remainingPath);
    ResolvedPath findPathInLinkMsg(const H5ObjectHeader& parentEntry,
                                   const H5LinkMsg& linkMsg,
                                   const H5Path& remainingPath);
};

#endif  // PATH_RESOLVER_V2_H
