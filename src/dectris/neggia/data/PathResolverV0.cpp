// SPDX-License-Identifier: MIT

#include "PathResolverV0.h"
#include <assert.h>
#include <memory>
#include <stdexcept>
#include "H5BTreeVersion2.h"
#include "H5FractalHeap.h"
#include "H5LocalHeap.h"
#include "constants.h"

PathResolverV0::PathResolverV0(const H5SymbolTableEntry& root) : _root(root) {}

ResolvedPath PathResolverV0::resolve(const H5Path& path) {
    return resolvePathInSymbolTableEntry(_root, path);
}

ResolvedPath PathResolverV0::resolvePathInSymbolTableEntry(
        const H5SymbolTableEntry& in,
        const H5Path& path) {
    assert(in.cacheType() == H5SymbolTableEntry::DATA ||
           in.cacheType() ==
                   H5SymbolTableEntry::GROUP);  // makes sense only for groups
                                                // (1) or objects that contain
                                                // links via objectheader (cache
                                                // type = 0, 1)
    H5SymbolTableEntry parentEntry(path.isAbsolute() ? _root : in);

    for (auto itemIterator = path.begin(); itemIterator != path.end();
         ++itemIterator)
    {
        auto item = *itemIterator;
        if (parentEntry.cacheType() == H5SymbolTableEntry::GROUP) {
            H5SymbolTableEntry stEntry;
            try {
                stEntry = parentEntry.find(item);
            } catch (const std::out_of_range&) {
                return findPathInObjectHeader(parentEntry, item,
                                              H5Path(path, itemIterator + 1));
            }
            if (stEntry.cacheType() == H5SymbolTableEntry::LINK) {
                return findPathInScratchSpace(parentEntry, stEntry,
                                              H5Path(path, itemIterator + 1));
            } else {
                parentEntry = stEntry;
                continue;
            }
        } else {
            throw std::runtime_error(
                    "Expected GROUP (cache_type = 1) at path item " + item);
        }
    }
    auto output = ResolvedPath{};
    output.objectHeader = parentEntry.objectHeader();
    return output;
}

ResolvedPath PathResolverV0::findPathInScratchSpace(
        H5SymbolTableEntry parentEntry,
        H5SymbolTableEntry symbolTableEntry,
        const H5Path& remainingPath) {
    size_t targetNameOffset = symbolTableEntry.getOffsetToLinkValue();
    H5LocalHeap treeHeap =
            H5Object(_root.fileAddress(), parentEntry.getAddressOfHeap());
    H5Path targetPath(treeHeap.data(targetNameOffset));
    return resolvePathInSymbolTableEntry(parentEntry,
                                         targetPath + remainingPath);
}

ResolvedPath PathResolverV0::findPathInLinkMsg(
        const H5SymbolTableEntry& parentEntry,
        const H5LinkMsg& linkMsg,
        const H5Path& remainingPath) {
    if (linkMsg.linkType() == H5LinkMsg::SOFT) {
        H5Path targetPath(linkMsg.targetPath());
        return resolvePathInSymbolTableEntry(parentEntry,
                                             targetPath + remainingPath);
    } else if (linkMsg.linkType() == H5LinkMsg::EXTERNAL) {
        std::string targetFile = linkMsg.targetFile();
        H5Path targetPath(linkMsg.targetPath());
        auto output = ResolvedPath{};
        output.externalFile.reset(new ResolvedPath::ExternalFile{
                targetFile, targetPath + remainingPath});
        return output;
    }
    throw std::runtime_error("unknown link type" +
                             std::to_string(linkMsg.linkType()));
}

uint32_t PathResolverV0::getFractalHeapOffset(
        const H5LinkInfoMsg& linkInfoMsg,
        const std::string& pathItem) const {
    size_t btreeAddress = linkInfoMsg.getBTreeAddress();
    if (btreeAddress == H5_INVALID_ADDRESS) {
        throw std::out_of_range("Invalid address");
    }
    H5BTreeVersion2 btree(_root.fileAddress(), btreeAddress);
    H5Object heapRecord(_root.fileAddress(), btree.getRecordAddress(pathItem));
    return heapRecord.read_u32(5);
}

ResolvedPath PathResolverV0::findPathInObjectHeader(
        const H5SymbolTableEntry& parentEntry,
        const std::string pathItem,
        const H5Path& remainingPath) {
    H5ObjectHeader objectHeader = parentEntry.objectHeader();
    for (size_t i = 0; i < objectHeader.numberOfMessages(); ++i) {
        auto msg = objectHeader.headerMessage(i);
        switch (msg.type) {
            case H5LinkMsg::TYPE_ID: {
                H5LinkMsg linkMsg(msg.object);
                if (linkMsg.linkName() != pathItem)
                    continue;
                return findPathInLinkMsg(parentEntry, linkMsg, remainingPath);
            }
            case H5LinkInfoMsg::TYPE_ID: {
                uint32_t heapOffset;
                H5LinkInfoMsg linkInfoMsg(msg.object);
                try {
                    heapOffset = getFractalHeapOffset(linkInfoMsg, pathItem);
                } catch (const std::out_of_range&) {
                    continue;
                }
                H5FractalHeap fractalHeap(_root.fileAddress(),
                                          linkInfoMsg.getFractalHeapAddress());
                H5LinkMsg linkMsg(fractalHeap.getHeapObject(heapOffset));
                assert(linkMsg.linkName() == pathItem);
                return findPathInLinkMsg(parentEntry, linkMsg, remainingPath);
            }
        }
    }
    throw std::out_of_range("could not find " + pathItem);
}
