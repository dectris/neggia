/**
MIT License

Copyright (c) 2017 DECTRIS Ltd.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

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
                                        const std::string pathItem,
                                        const H5Path& remainingPath);
    uint32_t getFractalHeapOffset(const H5LinkInfoMsg& linkInfoMsg,
                                  const std::string& pathItem) const;
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
